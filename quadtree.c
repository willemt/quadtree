/*
 
Copyright (c) 2011, Willem-Hendrik Thiart
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include "iterator.h"
#include "tea_vec.h"
#include "quadtree.h"
#include "linked_list_queue.h"

typedef struct node_s node_t;

/* org is only meaningful if you're a leaf */
/* your a leaf when you have an item */
struct node_s
{
    node_t *kids[4];
    veci2_t org;
    void *item;
};

typedef struct
{
    node_t *root;
    int count;
    int w, h;
} quadtree_in_t;

#define in(x) ((quadtree_in_t*)x->in)

static void __qt_add(node_t * parent,
                     veci2_t org, int midx, int midy, void *item);

/**
 * @return box coordinates from corresponding child idx */
static void __box_by_childidx(int child_idx, int *midx, int *midy)
{
    switch (child_idx)
    {
    case 0:
        *midx -= *midx / 2;
        *midy -= *midy / 2;
        break;
    case 1:
        *midx += *midx / 2;
        *midy -= *midy / 2;
        break;
    case 2:
        *midx -= *midx / 2;
        *midy += *midy / 2;
        break;
    case 3:
        *midx += *midx / 2;
        *midy += *midy / 2;
        break;
    }
}

static int __is_leaf(node_t * node)
{
    return NULL != node->item;
}

/*----------------------------------------------------------------------------*/

#define X 0
#define Y 1

/**
 * @return 1 on intersect, 0 otherwise */
int __lines_intersect(const vec2_t a1,
                      const vec2_t a2, const vec2_t b1, const vec2_t b2)
{
    float denom, numera, numerb;

    denom = ((b2[Y] - b1[Y]) * (a2[X] - a1[X])) -
        ((b2[X] - b1[X]) * (a2[Y] - a1[Y]));
    numera = ((b2[X] - b1[X]) * (a1[Y] - b1[Y])) -
        ((b2[Y] - b1[Y]) * (a1[X] - b1[X]));
    numerb = ((a2[X] - a1[X]) * (a1[Y] - b1[Y])) -
        ((a2[Y] - a1[Y]) * (a1[X] - b1[X]));

    if (0.0f == denom)
    {
        if (0.0f == numera && 0.0f == numerb)
        {
            return 0;   /* coincident  */
        }
        return 0;       /* parallel */
    }

    float ua = numera / denom;

    float ub = numerb / denom;

    return (ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f);
}

/**
 * @return 1 on intersect, 0 otherwise */
int __line_box_intersect(const vec2_t la,
                         const vec2_t lb,
                         const vec2_t boxv, const float w, const float h)
{
    vec2_t topLeft = { boxv[X], boxv[Y] };
    vec2_t topRight = { boxv[X] + w, boxv[Y] };
    vec2_t bottomRight = { boxv[X] + w, boxv[Y] + h };
    vec2_t bottomLeft = { boxv[X], boxv[Y] + h };

    /*  start or end point within box.. */
    if (la[X] > topLeft[X] && la[X] < topRight[X] &&
        la[Y] < bottomLeft[Y] && la[Y] > topLeft[Y] &&
        lb[X] > topLeft[X] && lb[X] < topRight[X] &&
        lb[Y] < bottomLeft[Y] && lb[Y] > topLeft[Y])
        return 1;

    /*  ..or, one of the following edges intersect */
    if (__lines_intersect(la, lb, topLeft, topRight) ||
        __lines_intersect(la, lb, topRight, bottomRight) ||
        __lines_intersect(la, lb, bottomRight, bottomLeft) ||
        __lines_intersect(la, lb, bottomLeft, topLeft))
        return 1;

    return 0;
}

/*----------------------------------------------------------------------------*/

quadtree_t *quadtree_new(const int w, const int h)
{
    node_t *root;
    quadtree_t *qt;

    qt = calloc(1, sizeof(quadtree_in_t));
    root = qt->root = calloc(1, sizeof(node_t));
    qt->w = w;
    qt->h = h;
    vec2Set(root->org, w / 2, h / 2);
}

/**
 * @returns an index from 0 to 3 */
static int __get_child_idx(node_t * parent, veci2_t org, const int midx,
                           const int midy)
{
    if (org[0] <= midx)
    {
        if (org[1] <= midy)
        {
            return 0;
        }
        else
        {
            return 2;
        }
    }
    else
    {
        if (org[1] <= midy)
        {
            return 1;
        }
        else
        {
            return 3;
        }
    }
}

static node_t *__qt_get(node_t * node, veci2_t org, int midx, int midy)
{
    if (!node)
        return NULL;

#if 0
    int ii, nodes = 0;

    for (ii = 0; ii < 4; ii++)
        if (node->kids[ii])
            nodes++;

    printf("(%d,%d) nodes:%d %s\n",
           midx, midy, nodes, __is_leaf(node) ? "leaf" : "");
#endif

    /*  stop at leaf */
    if (__is_leaf(node))
    {
        assert(node->item);

        if (vec2Equal(node->org, org))
        {
            return node;
        }
        else
        {
            return NULL;        /* didn't find it */
        }
    }
    else
    {
        int child_idx;

        assert(!node->item);

        child_idx = __get_child_idx(node, org, midx, midy);

        __box_by_childidx(child_idx, &midx, &midy);

        return __qt_get(node->kids[child_idx], org, midx, midy);
    }
}

/**
 * get item based off this origin */
void *quadtree_get(quadtree_t * qt, veci2_t org)
{
    node_t *node;

    node = __qt_get(qt->root, org, qt->w / 2, qt->h / 2);

    return (void *) node->item;
}

#if 0
static void *__qt_remove(node_t * node, veci2_t org, int midx, int midy)
{
    if (!node->kids)
    {
        if (__is_leaf(node) &&
            (node->org[0] == org[0] && node->org[1] == org[1]))
        {
            return node;
        }
        return NULL;
    }

    assert(node->kids && !__is_leaf(node));

    int child_idx = __get_child_idx(node, org, midx, midy);

    __coord_by_childidx(child_idx, &midx, &midy);

    node_t *child = __qt_remove(node->kids[child_idx], org, midx, midy);

    if (child)
    {
        void *item = child->item;

        tea_free(node->[child_idx]);
        node->[child_idx] = NULL;
    }
    return child;
}

void quadtree_remove(quadtree_t * qt, veci2_t org)
{

}
#endif

static node_t *__new_leaf(const veci2_t org, void *item)
{
    node_t *node;

    node = calloc(1, sizeof(node_t));
    node->item = item;
    vec2Copy(org, node->org);
    return node;
}

/*----------------------------------------------------------------------------*/

static int __node_num_children(node_t * node)
{
    int ii, num;

    for (ii = 0, num = 0; ii < 4; ii++)
    {
        if (node->kids[ii])
        {
            num++;
        }
    }
    return num;
}

static int __node_num_children_recurse(node_t * node)
{
    int ii, num;

    for (ii = 0, num = 0; ii < 4; ii++)
    {
        if (node->kids[ii])
        {
            if (__is_leaf(node->kids[ii]))
            {
                num++;
            }
            else
            {
                num += __node_num_children_recurse(node->kids[ii]);
            }
        }
    }
    return num;
}

/*----------------------------------------------------------------------------*/
static void __split_leaf(node_t * node, int midx, int midy)
{
    /* make the current node a child */
    veci2_t org_tmp;
    void *item_tmp;

    item_tmp = node->item;

    vec2Copy(node->org, org_tmp);
    node->item = NULL;
    vec2Clear(node->org);
    __qt_add(node, org_tmp, midx, midy, item_tmp);
    assert(__node_num_children(node) == 1);
}

static void
__qt_add(node_t * parent, veci2_t org, int midx, int midy, void *item)
{
    int child_idx;

    node_t *child;

    child_idx = __get_child_idx(parent, org, midx, midy);
    child = parent->kids[child_idx];

    if (child)
    {
        /*  stop at leaf */
        if (__is_leaf(child))
        {
            assert(__node_num_children(child) == 0);
            if (vec2Equal(child->org, org))
            {
                child->item = item;
            }
            else
            {
                __box_by_childidx(child_idx, &midx, &midy);
                __split_leaf(child, midx, midy);
                assert(__node_num_children(child) == 1);
                __qt_add(child, org, midx, midy, item);
                assert(__node_num_children_recurse(child) == 2);
            }
        }
        else
        {
            /*  recurse  */
            assert(!child->item);
            __box_by_childidx(child_idx, &midx, &midy);
            __qt_add(child, org, midx, midy, item);
        }
    }
    else
    {
        /*  stop at new leaf */
        child = parent->kids[child_idx] = __new_leaf(org, item);
    }
}

void quadtree_add(quadtree_t * qt, veci2_t org, void *item)
{
    assert(item);
    assert(qt->root);
    __qt_add(qt->root, org, qt->w / 2, qt->h / 2, item);
}

/*----------------------------------------------------------------------------*/
#if 0
static void __view(node_t * node, int midx, int midy)
{
    if (!node)
    {
        return;
    }
    else if (__is_leaf(node))
    {
        printf("(%d,%d)", node->org[0], node->org[1]);
    }
    else
    {
        int ii;

        printf("[<%d,%d>", midx, midy);
        for (ii = 0; ii < 4; ii++)
        {
            if (node->kids[ii])
            {
                int tmidx = midx, tmidy = midy;

                __box_by_childidx(ii, &tmidx, &tmidy);
                __view(node->kids[ii], tmidx, tmidy);
            }
            printf(",");
        }
        printf("],");
    }
}

void quadtree_view(quadtree_t * qt)
{
    __view(qt->root, qt->w / 2, qt->h / 2);
    printf("\n");
}

#endif
/*----------------------------------------------------------------------------*/

static int __qt_count(node_t * node)
{
    int ii;
    int count = 0;

    if (!node)
        return 0;

    if (__is_leaf(node))
    {
        for (ii = 0; ii < 4; ii++)
            assert(!node->kids[ii]);
        return 1;
    }

    for (ii = 0; ii < 4; ii++)
        count += __qt_count(node->kids[ii]);

    return count;
}

int quadtree_count(const quadtree_t * qt)
{
//      assert(qt->count == __qt_count(qt->root));
//      return qt->count;
    return __qt_count(qt->root);
}

/*----------------------------------------------------------------------------*/
iterator_t *quadtree_iterator(quadtree_t * qt)
{
    rect_t rect = {.x = 0,.y = 0,.w = qt->w,.h = qt->h };
    return quadtree_iterator_from_bbox(qt, &rect);
}

/*----------------------------------------------------------------------------*/

#define tea_bbox_point(x1,y1,x2,y2,w,h)\
	((x1) <= (x2) + (w) && (y1) <= (y2) + (h) && (x1) >= (x2) && (y1) >= (y2))

/**
 * Add nodes from bounding box into a queue
 * */
static void
__enqueue_nodes_from_bbox(node_t * node,
                          linked_list_queue_t * qu,
                          rect_t * rect, int midx, int midy)
{
    if (__is_leaf(node))
    {
        if (tea_bbox_point(node->org[X], node->org[Y],
                           rect->x, rect->y, rect->w, rect->h))
        {
            llqueue_offer(qu, node->item);
        }
        return;
    }

    int ii;

    for (ii = 0; ii < 4; ii++)
    {
        if (!node->kids[ii])
            continue;

#if 1
        /* cull nodes that are irrelevant to the bounding box */

        if ((0 == ii || 2 == ii) && midx < rect->x)
            continue;
        if ((1 == ii || 3 == ii) && midx > rect->x + rect->w)
            continue;
        if ((0 == ii || 1 == ii) && midy < rect->y)
            continue;
        if ((2 == ii || 3 == ii) && midy > rect->y + rect->h)
            continue;
#endif

        /* check that node */

        int midx_tmp = midx, midy_tmp = midy;

        __box_by_childidx(ii, &midx_tmp, &midy_tmp);
        __enqueue_nodes_from_bbox(node->kids[ii], qu, rect, midx_tmp, midy_tmp);
    }
}

static int __has_next_bbox(void *data)
{
    iterator_t *iter = data;
    linked_list_queue_t *qu = iter->data;

    assert(qu);

    if (0 == llqueue_count(qu))
    {
        if (iter->next_iter)
        {
            void *data;

            data = iter->data;
            iter->data = iter->next_iter->data;
            iter->next_iter->data = data;
            iter->next_iter->done(iter->next_iter);
            iter->next_iter = NULL;
        }
        else
            return 0;
    }

    return 1;
}

static void *__next_bbox(void *data)
{
    iterator_t *iter = data;
    linked_list_queue_t *qu = iter->data;
    void *item;

    if (!__has_next_bbox(iter))
        return NULL;

    assert(qu);
    item = llqueue_poll(qu);
    assert(item);

    return item;
}

static void __done_bbox(void *data)
{
    iterator_t *iter = data;

    while (llqueue_poll(iter->data));
    llqueue_free(iter->data);
    free(iter);
}

iterator_t *quadtree_iterator_from_bbox(quadtree_t * qt, const rect_t * rect)
{
    rect_t rect_tmp;
    linked_list_queue_t *qu;
    iterator_t *iter;

    qu = llqueue_new();
    iter = calloc(1, sizeof(iterator_t));
    iter->next = __next_bbox;
    iter->has_next = __has_next_bbox;
    iter->done = __done_bbox;
    iter->data = qu;

    /* this assumption will make things easier later on? */

    rect_tmp.w = rect->w;
    rect_tmp.h = rect->h;

    if (rect->x > qt->w)
    {
        rect_tmp.x = qt->w;
    }
    else
    {
        rect_tmp.x = rect->x < 0 ? 0 : rect->x;
    }

    if (rect->y > qt->h)
    {
        rect_tmp.y = qt->h;
    }
    else
    {
        rect_tmp.y = rect->y < 0 ? 0 : rect->y;
    }

    __enqueue_nodes_from_bbox(qt->root, qu, &rect_tmp, qt->w / 2, qt->h / 2);

    return iter;
}

/**
 * offer items to a queue */
static void
__enqueue_nodes_from_line(node_t * node,
                          linked_list_queue_t * qu,
                          float itemw,
                          const vec2_t v1,
                          const vec2_t v2, int midx, int midy, float w, float h)
{
    int ii;
    vec2_t org = { (float) midx - w, (float) midy - h };

#if 1
    if (w > itemw || h > itemw)
        if (!__line_box_intersect(v1, v2, org, w, h))
            return;
#endif

    /* stop at leaf */
    if (__is_leaf(node))
    {
        vec2Copy(node->org, org);
        if (__line_box_intersect(v1, v2, org, itemw, itemw))
        {
            llqueue_offer(qu, node->item);
        }
        return;
    }

    /* check all four kids */
    for (ii = 0; ii < 4; ii++)
    {
        if (!node->kids[ii])
            continue;

        /* check that node */

        int midx_tmp = midx, midy_tmp = midy;

        __box_by_childidx(ii, &midx_tmp, &midy_tmp);

        float w = fabs(midx_tmp - midx);
        float h = fabs(midy_tmp - midy);

        __enqueue_nodes_from_line(node->kids[ii], qu, itemw,
                                  v1, v2, midx_tmp, midy_tmp, w, h);
    }
}

/**
 * Create an iterator from an intersecting line. */
iterator_t *quadtree_iterator_from_line(quadtree_t * qt,
                                        const vec2_t v1, const vec2_t v2,
                                        float itemw)
{
    linked_list_queue_t *qu;
    iterator_t *iter;

    qu = llqueue_new();

    iter = calloc(1, sizeof(iterator_t));

    iter->next = __next_bbox;
    iter->has_next = __has_next_bbox;
    iter->done = __done_bbox;
    iter->data = qu;

    __enqueue_nodes_from_line(qt->root, qu, itemw,
                              v1, v2, qt->w / 2, qt->h / 2,
                              ((float) qt->w / 2), (float) (qt->h / 2));

    return iter;
}

/*--------------------------------------------------------------79-characters-*/

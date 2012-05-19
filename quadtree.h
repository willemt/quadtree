typedef struct
{
    void *root;
    void *data;
    int w, h;
} quadtree_t;

iterator_t *quadtree_iterator_from_bbox(quadtree_t * qt, const rect_t * rect);

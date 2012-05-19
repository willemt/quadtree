#ifndef STORE_ITERATOR_H
#define STORE_ITERATOR_H

/**
 * POLICY:
 * getting an iterator from a NULL data structure will return an empty iterator */

typedef struct _iter iterator_t;

typedef void *(*iterator_next_func) (void *, iterator_t *);

typedef void *(*iteratorhas_next_func) (void *, iterator_t *);

struct _iter
{
    int current;
    void *data;                 /* Data for callback funcs to use */
    void *(*next) (void *);     /* Gets the next item */
    int (*has_next) (void *);   /* Checks if the next item exists */
    const void *(*next_const) (void *); /* Gets the next item */
    void (*done) (void *);      /* Cleanup */
    unsigned int (*len) (const void *);
    void *(*reverse) (void *);
    iterator_t *next_iter;
    void *(*peek) (void *);     /* Gets the next item */
};

typedef iterator_t *(*tea_adt_iter_f) (void *ds);

void *iterator_next(iterator_t * iter);

const void *iterator_next_const(iterator_t * iter);

int iterator_has_next(iterator_t * iter);

int iterator_has_next_done(iterator_t * iter);

void iterator_done(iterator_t * iter);

void *iterator_peek(iterator_t * iter);

void iterator_forall(iterator_t * iter, void (*op) (void *object));

void iterator_forall_udata(iterator_t * iter,
                           void (*op) (void *object, void *udata), void *udata);

#define iterator(x) ((iterator_t*)(x))

#endif /* STORE_ITERATOR_H */

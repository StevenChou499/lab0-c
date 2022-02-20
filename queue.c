#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head) {
        INIT_LIST_HEAD(head);
        return head;
    }
    free(head);
    return NULL;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (l) {
        struct list_head *tmp = l->next;
        while (tmp != l) {
            element_t *del_el;
            del_el = container_of(tmp, element_t, list);
            tmp = tmp->next;
            free(del_el->value);
            free(del_el);
        }
        free(l);
    }
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_el = malloc(sizeof(element_t));
    if (new_el && s) {
        new_el->value = malloc(strlen(s) + 1);
        if (new_el->value) {
            strncpy(new_el->value, s, strlen(s));
            *(new_el->value + strlen(s)) = '\0';
            list_add(&new_el->list, head);
            return true;
        }
        free(new_el->value);
    }
    free(new_el);
    return false;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new_el = malloc(sizeof(element_t));
    if (new_el && s) {
        new_el->value = malloc(strlen(s) + 1);
        if (new_el->value) {
            strncpy(new_el->value, s, strlen(s));
            *(new_el->value + strlen(s)) = '\0';
            list_add_tail(&new_el->list, head);
            return true;
        }
        free(new_el->value);
    }
    free(new_el);
    return false;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head && !list_empty(head)) {
        element_t *rem_el = container_of(head->next, element_t, list);
        int char_len = strlen(rem_el->value) < bufsize - 1
                           ? strlen(rem_el->value)
                           : bufsize - 1;
        if (sp) {
            sp = realloc(sp, char_len + 1);
            strncpy(sp, rem_el->value, char_len + 1);
            *(sp + char_len) = '\0';
        }
        list_del(&rem_el->list);
        return rem_el;
    }
    return NULL;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head && !list_empty(head)) {
        element_t *rem_el = container_of(head->prev, element_t, list);
        int char_len = strlen(rem_el->value) < bufsize - 1
                           ? strlen(rem_el->value)
                           : bufsize - 1;
        if (sp) {
            sp = realloc(sp, char_len + 1);
            strncpy(sp, rem_el->value, char_len + 1);
            *(sp + char_len) = '\0';
        }
        list_del(&rem_el->list);
        return rem_el;
    }
    return NULL;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (head && !list_empty(head)) {
        int n = q_size(head) % 2 == 0 ? q_size(head) / 2 : q_size(head) / 2 + 1;
        struct list_head *tmp = head;
        for (int i = 0; i != n; i++) {
            tmp = tmp->next;
        }
        element_t *del_el = container_of(tmp, element_t, list);
        list_del(tmp);
        free(del_el->value);
        free(del_el);
        return true;
    }
    return false;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (head && !list_empty(head) && !list_is_singular(head)) {
        struct list_head *tmp1 = head->next;
        struct list_head *tmp2 = tmp1->next;
        struct list_head *tmp = NULL;
        while (tmp1 != head) {
            element_t *del_el_1 = container_of(tmp1, element_t, list);
            element_t *del_el_2 = container_of(tmp2, element_t, list);
            while (!strcmp(del_el_1->value, del_el_2->value)) {
                tmp = tmp1;
                printf("%s, %s\n", del_el_1->value, del_el_2->value);
                list_del(tmp2);
                tmp2 = tmp1->next;
                free(del_el_2->value);
                free(del_el_2);
                del_el_2 = container_of(tmp2, element_t, list);
                if (tmp2 == head)
                    break;
            }
            tmp1 = tmp2;
            tmp2 = tmp1->next;
            if (tmp) {
                element_t *del_el_3 = container_of(tmp, element_t, list);
                list_del(tmp);
                free(del_el_3->value);
                free(del_el_3);
                tmp = NULL;
            }
        }
        return true;
    }
    return false;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    if (head) {
        struct list_head *first = head->next;
        struct list_head *second = first->next;
        for (int i = 0; i < q_size(head) / 2; i++) {
            element_t *f_node = container_of(first, element_t, list);
            element_t *s_node = container_of(second, element_t, list);
            char *tmp = f_node->value;
            f_node->value = s_node->value;
            s_node->value = tmp;
            first = first->next->next;
            second = first->next;
        }
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (head) {
        struct list_head *fward = head->next;
        struct list_head *bward = head->prev;
        for (int i = 0; i < q_size(head) / 2; i++) {
            element_t *swap_f = container_of(fward, element_t, list);
            element_t *swap_b = container_of(bward, element_t, list);
            char *tmp = swap_f->value;
            swap_f->value = swap_b->value;
            swap_b->value = tmp;
            fward = fward->next;
            bward = bward->prev;
        }
    }
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    struct list_head less, greater;
    element_t *pivot, *item, *is = NULL;

    if (!head || list_empty(head) || list_is_singular(head))
        return;

    INIT_LIST_HEAD(&less);
    INIT_LIST_HEAD(&greater);

    pivot = list_first_entry(head, element_t, list);
    list_del(&pivot->list);

    list_for_each_entry_safe (item, is, head, list) {
        if (strcmp(item->value, pivot->value) < 0)
            list_move_tail(&item->list, &less);
        else
            list_move(&item->list, &greater);
    }

    q_sort(&less);
    q_sort(&greater);

    list_add(&pivot->list, head);
    list_splice(&less, head);
    list_splice_tail(&greater, head);
}
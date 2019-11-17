#include "headers.h"
// --- BEGIN UNIT TEST FUNCTIONS ---
void cache_init_test() {
  cache_queue_t *cache_queue = new_cache_queue(LRU_MAX_SLOT);
  printf("max slot: %d\n", cache_queue->max_slot);
  cache_node_t *cache_node = cache_queue->front;
  int count = 0;
  while (cache_node != NULL) {
    count += 1;
    print_cache_node(cache_node);
    cache_node = cache_node->next;
  }
  free_cache_queue(&cache_queue);
  printf("is cache queue null?%d\n", cache_queue == NULL);
  // printf("is cache queue front node null?%d\n", cache_queue->front == NULL);
}
void cache_enqueue_test() {
  cache_queue_t *cache_queue = new_cache_queue(LRU_MAX_SLOT);
  cache_node_t *new_node = new_cache_node(NULL, NULL, 100);
  print_cache_node(new_node);
  char document[] = "hello world";
  memcpy(new_node->buffer, document, strlen(document));
  cache_enqueue(cache_queue, new_node);
  printf("new node buffer: %s\n", cache_queue->front->buffer);
  print_cache_queue(cache_queue);
  return;
}
// --- END UNIT TEST FUNCTIONS ---
void cache_eviction_test() {
  cache_queue_t *cache_queue = new_cache_queue(LRU_MAX_SLOT);
  cache_node_t *new_node;
  char document[] = "hello world";

  // hold address to front node after eviction
  new_node = new_cache_node(NULL, NULL, 50);
  new_node->status = IN_USE;
  memcpy(new_node->buffer, document, strlen(document));
  cache_enqueue(cache_queue, new_node);
  cache_node_t *evicted_node = cache_queue->front;
  printf("address before eviction: %p\n", evicted_node);
  print_cache_node(evicted_node);

  for (int i = 0; i < 13; i++) {
    new_node = new_cache_node(NULL, NULL, 100 + i);
    memcpy(new_node->buffer, document, strlen(document));
    cache_enqueue(cache_queue, new_node);
    // print_cache_node(evicted_node);
  }

  print_cache_queue(cache_queue);
  printf("address after eviction: %p\n", evicted_node);
  print_cache_node(evicted_node);
  return;
}

http_err_t main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: ./proxy <ip to bind> <port to bind>\n");
    return HTTP_FAIL;
  }
  // cache_init_test();
  // cache_enqueue_test();
  cache_eviction_test();
  return HTTP_OK;
}

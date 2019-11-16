#include "headers.h"

http_err_t main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: ./proxy <ip to bind> <port to bind>\n");
    return HTTP_FAIL;
  }
  cache_queue_t *cache_queue = new_cache_queue(LRU_MAX_SLOT);
  printf("%d\n", cache_queue->max_slot);
  cache_node_t *cache_node = cache_queue->front;
  int count = 0;
  while (cache_node != NULL) {
    count += 1;
    printf("node: %d, buffer size:%ld\n", count, cache_node->buffer_size);
    cache_node = cache_node->next;
  }
  free_cache_queue(&cache_queue);
  printf("is cache queue null?%d\n", cache_queue == NULL);

  printf("true: %d\n", 1 == 1);
  return HTTP_OK;
}

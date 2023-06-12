#include "thread.h"
void* thread_func(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg;

    while (true) {
        task_t task;

        pthread_mutex_lock(&pool->mutex);

        while (pool->is_empty && !pool->is_shutdown) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }

        if (pool->is_shutdown) {
            pthread_mutex_unlock(&pool->mutex);
            pthread_exit(NULL);
        }

        task.priority = TASK_PRIORITY_NORMAL;
        task.func = NULL;
        task.arg = NULL;

        if (!queue_is_empty(&pool->task_queue)) {
            queue_pop(&pool->task_queue, &task);
        }

        if (queue_is_empty(&pool->task_queue)) {
            pool->is_empty = true;
        }

        if (queue_is_full(&pool->task_queue)) {
            pool->is_full = true;
        }

        pthread_cond_broadcast(&pool->cond);

        pthread_mutex_unlock(&pool->mutex);

        if (task.func) {
            task.func(task.arg);
        }
    }

    return NULL;
}

int compare_task_priority(const void* a, const void* b) {
    task_t* t1 = (task_t*)a;
    task_t* t2 = (task_t*)b;

    return t1->priority - t2->priority;
}

int get_idle_thread_count(thread_pool_t* pool) {
    int count = 0;

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_tryjoin_np(pool->threads[i], NULL) == 0) {
            count++;
        }
    }

    return count;
}

int cancel_task_in_thread_pool(thread_pool_t* pool, void (*func)(void*),
                               void* arg) {
    int num_cancelled = 0;

    pthread_mutex_lock(&pool->mutex);

    for (int i = pool->head; i != pool->tail; i = (i + 1) % THREAD_POOL_SIZE) {
        if (pool->task_queue[i].func == func &&
            pool->task_queue[i].arg == arg) {
            for (int j = i; j != pool->head;
                 j = (j - 1 + THREAD_POOL_SIZE) % THREAD_POOL_SIZE) {
                pool->task_queue[j] =
                    pool->task_queue[(j - 1 + THREAD_POOL_SIZE) %
                                     THREAD_POOL_SIZE];
            }

            pool->head = (pool->head + 1) % THREAD_POOL_SIZE;
            pool->count--;
            num_cancelled++;
        }
    }

    if (num_cancelled > 0) {  // 唤醒工作线程
        pool->is_full = false;
        pool->is_empty = (pool->count == 0);

        pthread_cond_broadcast(&pool->cond);
    }

    pthread_mutex_unlock(&pool->mutex);

    return num_cancelled;
}

thread_pool_t* thread_pool_init(int thread_count, int queue_size) {
    thread_pool_t* pool = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    pool->thread_count = thread_count;
    pool->queue_size = queue_size;
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    queue_init(&pool->task_queue, queue_size);
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
    pool->is_shutdown = false;
    pool->is_empty = true;
    pool->is_full = false;

    int i;
    for (i = 0; i < thread_count; ++i) {
        pthread_create(&pool->threads[i], NULL, thread_func, (void*)pool);
    }

    return pool;
}

int init_thread_pool(thread_pool_t* pool, int max_task_priority,
                     int max_thread_concurrency, int idle_thread_timeout) {
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    pool->is_full = false;
    pool->is_empty = true;
    pool->max_task_priority = max_task_priority;
    pool->max_thread_concurrency = max_thread_concurrency;
    pool->idle_thread_timeout = idle_thread_timeout;
    // pool->is_shutdown = false;
    long

        pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);

    // for (int i = 0; i < THREAD_POOL_SIZE; i++) {
    // pthread_create(&pool->threads[i], NULL, thread_func, pool);
    // }

    return 0;
}

int stop_thread_pool(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->mutex);

    // pool->is_shutdown = true;
    pool->is_empty = false;
    pool->is_full = false;

    pthread_cond_broadcast(&pool->cond);

    pthread_mutex_unlock(&pool->mutex);

    // 等待工作线程退出
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);

    return 0;
}

bool add_task_to_thread_pool(thread_pool_t* pool, task_func_t func, void* arg,
                             int priority) {
    if (pool == NULL || func == NULL) {
        return false;
    }

    task_t task;
    task.func = func;
    task.arg = arg;
    task.priority = priority;

    pthread_mutex_lock(&pool->mutex);

    while (queue_is_full(&pool->task_queue)) {
        pthread_cond_wait(&pool->cond, &pool->mutex);
    }

    if (queue_push(&pool->task_queue, &task)) {
        pool->is_empty = false;
        pool->is_full = queue_is_full(&pool->task_queue);
        pthread_cond_broadcast(&pool->cond);
    }

    pthread_mutex_unlock(&pool->mutex);

    return true;
}

int main() {
    thread_pool_t* pool = thread_pool_init(4, 16);

    int i;
    for (i = 0; i < 32; ++i) {
        int* arg = (int*)malloc(sizeof(int));
        *arg = i;
        add_task_to_thread_pool(pool, (task_func_t)printf, arg,
                                TASK_PRIORITY_NORMAL);
    }

    thread_pool_destroy(pool);

    return 0;
}

thread_pool_t* thread_pool_init(int thread_count, int queue_size) {
    if (thread_count <= 0 || queue_size <= 0) {
        return NULL;
    }

    thread_pool_t* pool = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    pool->thread_count = thread_count;
    pool->queue_size = queue_size;
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count);
    queue_init(&pool->task_queue, queue_size);
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
    pool->is_shutdown = false;
    pool->is_empty = true;
    pool->is_full = false;

    int i;
    for (i = 0; i < thread_count; ++i) {
        pthread_create(&pool->threads[i], NULL, thread_func, (void*)pool);
    }

    return pool;
}

void thread_pool_destroy(thread_pool_t* pool) {
    if (pool == NULL) {
        return;
    }

    pool->is_shutdown = true;

    pthread_cond_broadcast(&pool->cond);

    int i;
    for (i = 0; i < pool->thread_count; ++i) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    queue_destroy(&pool->task_queue);
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);

    free(pool);
}

bool add_task_to_thread_pool(thread_pool_t* pool, task_func_t func, void* arg,
                             int priority) {
    if (pool == NULL || func == NULL) {
        return false;
    }

    task_t task;
    task.func = func;
    task.arg = arg;
    task.priority = priority;

    pthread_mutex_lock(&pool->mutex);

    while (queue_is_full(&pool->task_queue)) {
        pthread_cond_wait(&pool->cond, &pool->mutex);
    }

    if (queue_push(&pool->task_queue, &task)) {
        pool->is_empty = false;
        pool->is_full = queue_is_full(&pool->task_queue);
        pthread_cond_broadcast(&pool->cond);
    }

    pthread_mutex_unlock(&pool->mutex);

    return true;
}

void* thread_func(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg;

    while (true) {
        task_t task;

        pthread_mutex_lock(&pool->mutex);

        while (pool->is_empty && !pool->is_shutdown) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }

        if (pool->is_shutdown) {
            pthread_mutex_unlock(&pool->mutex);
            pthread_exit(NULL);
        }

        task.priority = TASK_PRIORITY_NORMAL;
        task.func = NULL;
        task.arg = NULL;

        if (!queue_is_empty(&pool->task_queue)) {
            queue_pop(&pool->task_queue, &task);
        }

        if (queue_is_empty(&pool->task_queue)) {
            pool->is_empty = true;
        }

        if (queue_is_full(&pool->task_queue)) {
            pool->is_full = true;
        }

        pthread_cond_broadcast(&pool->cond);

        pthread_mutex_unlock(&pool->mutex);

        if (task.func) {
            task.func(task.arg);
        }
    }

    return NULL;
}
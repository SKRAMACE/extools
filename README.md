Memory management library designed for simplicity.

INSTALLATION:
    make
    sudo make install

USAGE:
    // C Source file
    #include <radpool.h>

    # Makefile
    -lradpool

API:
    // Create a new top-level pool
    POOL *create_pool();

    // Create a pool within an existing pool
    POOL *create_subpool(POOL *pool);

    // Copy all contents and subpools into the target pool
    POOL *copy_pool(POOL *pool);

    // Allocate space from the target pool
    void *palloc(POOL *pool, size_t bytes);

    // Allocate and zero space from the target pool
    void *pcalloc(POOL *pool, size_t bytes);

    // Change size of target buffer, copy old buffer, and update pool records
    void *repalloc(void *addr, size_t bytes, POOL *pool);

    // Free all memory in pool and subpools
    void pfree(POOL *pool);

    // Free all memory in all pools
    void pool_cleanup();

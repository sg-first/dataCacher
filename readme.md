dataCacher
============
Internal and external storage scheduling system for IO intensive database.

How to use
------------
* Implementation Interface functions for external memory access (in `IOinter.h`). In this process, you need to address your external storage file structure so that you can access an data entry through `blockNum` and `wordaddress`.
* Include `cache.h`, create cache object(Specify the template parameter as the data type of your data entry (generally `string`)). Call the cache method to access the data and enjoy the memory cache acceleration effect.

Acceleration strategy
-----------------
The system will load the accessed external memory entry into memory as a cache unit. When a read operation occurs, it will first find out whether there is a cache unit that has cached the entry. If hit, it can be read directly in memory. On this basis, we introduce matching replacement strategies and write acceleration strategies. You can choose the right strategy based on the business environment:
### Cache Type
* Fully associative mapping
* Direct mapping
### Replacement strategy
* Least frequently used replacement(LFU)
* Least recently used replacement(LRU)
* Random replacement
### Writing strategy
* Write back strategy
* All write strategy
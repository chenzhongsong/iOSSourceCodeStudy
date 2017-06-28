/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import <Foundation/Foundation.h>
#import "SDWebImageCompat.h"
#import "SDImageCacheConfig.h"

/**
 缓存的类型、内存、磁盘、不缓存。

 - but: <#but description#>
 */
typedef NS_ENUM(NSInteger, SDImageCacheType) {
    /** 图像不可用sdwebimage缓存，但从网上下载。
     * The image wasn't available the SDWebImage caches, but was downloaded from the web.
     */
    SDImageCacheTypeNone,
    /** 该图像是从磁盘缓存中获得的。
     * The image was obtained from the disk cache.
     */
    SDImageCacheTypeDisk,
    /** 该图像是从内存缓存中获得的。
     * The image was obtained from the memory cache.
     */
    SDImageCacheTypeMemory
};

/**
 缓存查询完成的Block

 @param image <#image description#>
 @param data <#data description#>
 @param cacheType <#cacheType description#>
 */
typedef void(^SDCacheQueryCompletedBlock)(UIImage * _Nullable image, NSData * _Nullable data, SDImageCacheType cacheType);

/**
 检查缓存完成Block
 
 @param isInCache <#isInCache description#>
 */
typedef void(^SDWebImageCheckCacheCompletionBlock)(BOOL isInCache);

/**
 计算大小Block
 
 @param fileCount <#fileCount description#>
 @param totalSize <#totalSize description#>
 */
typedef void(^SDWebImageCalculateSizeBlock)(NSUInteger fileCount, NSUInteger totalSize);


/** SDImageCache 维护 内存缓存和一个可选的磁盘高速缓存。磁盘缓存写操作是异步执行，所以不要添加不必要的等待to the UI。
 * SDImageCache maintains a memory cache and an optional disk cache. Disk cache write operations are performed
 * asynchronous so it doesn’t add unnecessary latency to the UI.
 */
@interface SDImageCache : NSObject

#pragma mark - Properties

/** 缓存配置对象——存储所有设置
 *  Cache Config object - storing all kind of settings
 */
@property (nonatomic, nonnull, readonly) SDImageCacheConfig *config;

/** 内存图像缓存的最大“总成本”。成本函数是存储在内存中的像素数。
 * The maximum "total cost" of the in-memory image cache. The cost function is the number of pixels held in memory.
 */
@property (assign, nonatomic) NSUInteger maxMemoryCost;

/** 缓存应该保存的最大对象数。
 * The maximum number of objects the cache should hold.
 */
@property (assign, nonatomic) NSUInteger maxMemoryCountLimit;

#pragma mark - Singleton and initialization 单例 和 初始化

/** 返回缓存实例单例
 * Returns global shared cache instance
 *  SDImageCache全局实例
 * @return SDImageCache global instance
 */
+ (nonnull instancetype)sharedImageCache;

/** 初始化一个新的缓存仓库用一个特殊的命名空间
 * Init a new cache store with a specific namespace
 *  用于此高速缓存存储的命名空间。
 * @param ns The namespace to use for this cache store
 */
- (nonnull instancetype)initWithNamespace:(nonnull NSString *)ns;

/** 初始化一个新的缓存仓库用一个特殊的命名空间和目录
 * Init a new cache store with a specific namespace and directory
 *  用于此高速缓存存储的命名空间。目录中的磁盘图像缓存
 * @param ns        The namespace to use for this cache store
 * @param directory Directory to cache disk images in
 */
- (nonnull instancetype)initWithNamespace:(nonnull NSString *)ns
                       diskCacheDirectory:(nonnull NSString *)directory NS_DESIGNATED_INITIALIZER;

#pragma mark - Cache paths
/**
 图片缓存目录
 
 @param fullNamespace 自定义目录
 @return 完整目录
 */
- (nullable NSString *)makeDiskCachePath:(nonnull NSString*)fullNamespace;

/** 添加一个只读缓存路径搜索图像预缓存的sdimagecache
 * Add a read-only cache path to search for images pre-cached by SDImageCache
 * Useful if you want to bundle pre-loaded images with your app
 *
 * @param path The path to use for this read-only cache path
 */
- (void)addReadOnlyCachePath:(nonnull NSString *)path;

#pragma mark - Store Ops 存储操作

/** 在给定的键中异步地将图像存储到内存和磁盘缓存中。
 * Asynchronously store an image into memory and disk cache at the given key.
 *
 * @param image           The image to store
 * @param key             The unique image cache key, usually it's image absolute URL
 * @param completionBlock A block executed after the operation is finished
 */
- (void)storeImage:(nullable UIImage *)image
            forKey:(nullable NSString *)key
        completion:(nullable SDWebImageNoParamsBlock)completionBlock;

/**
 * Asynchronously store an image into memory and disk cache at the given key.
 *
 * @param image           The image to store
 * @param key             The unique image cache key, usually it's image absolute URL
 * @param toDisk          Store the image to disk cache if YES
 * @param completionBlock A block executed after the operation is finished
 */
- (void)storeImage:(nullable UIImage *)image
            forKey:(nullable NSString *)key
            toDisk:(BOOL)toDisk
        completion:(nullable SDWebImageNoParamsBlock)completionBlock;

/**
 * Asynchronously store an image into memory and disk cache at the given key.
 *
 * @param image           The image to store
 * @param imageData       The image data as returned by the server, this representation will be used for disk storage
 *                        instead of converting the given image object into a storable/compressed image format in order
 *                        to save quality and CPU
 * @param key             The unique image cache key, usually it's image absolute URL
 * @param toDisk          Store the image to disk cache if YES
 * @param completionBlock A block executed after the operation is finished
 */
- (void)storeImage:(nullable UIImage *)image
         imageData:(nullable NSData *)imageData
            forKey:(nullable NSString *)key
            toDisk:(BOOL)toDisk
        completion:(nullable SDWebImageNoParamsBlock)completionBlock;

/**
 * Synchronously store image NSData into disk cache at the given key.
 *
 * @warning This method is synchronous, make sure to call it from the ioQueue
 *
 * @param imageData  The image data to store
 * @param key        The unique image cache key, usually it's image absolute URL
 */
- (void)storeImageDataToDisk:(nullable NSData *)imageData forKey:(nullable NSString *)key;

#pragma mark - Query and Retrieve Ops 查询和检索操作

/** 异步检查已经存在于磁盘高速缓存的图像（不加载图像）
 *  Async check if image exists in disk cache already (does not load the image)
 *
 *  @param key             the key describing the url
 *  @param completionBlock the block to be executed when the check is done.
 *  @note the completion block will be always executed on the main queue
 */
- (void)diskImageExistsWithKey:(nullable NSString *)key completion:(nullable SDWebImageCheckCacheCompletionBlock)completionBlock;

/**
 * Operation that queries the cache asynchronously and call the completion when done.
 *
 * @param key       The unique key used to store the wanted image
 * @param doneBlock The completion block. Will not get called if the operation is cancelled
 *
 * @return a NSOperation instance containing the cache op
 */
- (nullable NSOperation *)queryCacheOperationForKey:(nullable NSString *)key done:(nullable SDCacheQueryCompletedBlock)doneBlock;

/**
 * Query the memory cache synchronously.
 *
 * @param key The unique key used to store the image
 */
- (nullable UIImage *)imageFromMemoryCacheForKey:(nullable NSString *)key;

/**
 * Query the disk cache synchronously.
 *
 * @param key The unique key used to store the image
 */
- (nullable UIImage *)imageFromDiskCacheForKey:(nullable NSString *)key;

/**
 * Query the cache (memory and or disk) synchronously after checking the memory cache.
 *
 * @param key The unique key used to store the image
 */
- (nullable UIImage *)imageFromCacheForKey:(nullable NSString *)key;

#pragma mark - Remove Ops

/** 异步删除内存和磁盘缓存中的图像
 * Remove the image from memory and disk cache asynchronously
 *
 * @param key             The unique image cache key
 * @param completion      A block that should be executed after the image has been removed (optional)
 */
- (void)removeImageForKey:(nullable NSString *)key withCompletion:(nullable SDWebImageNoParamsBlock)completion;

/** 异步删除内存和可选的磁盘缓存中的图像
 * Remove the image from memory and optionally disk cache asynchronously
 *
 * @param key             The unique image cache key
 * @param fromDisk        Also remove cache entry from disk if YES
 * @param completion      A block that should be executed after the image has been removed (optional)
 */
- (void)removeImageForKey:(nullable NSString *)key fromDisk:(BOOL)fromDisk withCompletion:(nullable SDWebImageNoParamsBlock)completion;

#pragma mark - Cache clean Ops 缓存清理行动操作

/** 清除所有内存缓存的图像
 * Clear all memory cached images
 */
- (void)clearMemory;

/** 异步清除所有磁盘缓存图像。非阻塞方法-立即返回。
 * Async clear all disk cached images. Non-blocking method - returns immediately.
    在缓存过期完成后应该执行的块（可选）
 * @param completion    A block that should be executed after cache expiration completes (optional)
 */
- (void)clearDiskOnCompletion:(nullable SDWebImageNoParamsBlock)completion;

/** 异步删除所有过期缓存的图像从磁盘。非阻塞方法-立即返回。
 * Async remove all expired cached image from disk. Non-blocking method - returns immediately.
    在缓存过期完成后应该执行的块（可选）
 * @param completionBlock A block that should be executed after cache expiration completes (optional)
 */
- (void)deleteOldFilesWithCompletionBlock:(nullable SDWebImageNoParamsBlock)completionBlock;

#pragma mark - Cache Info

/** 获取磁盘缓存所使用的大小
 * Get the size used by the disk cache
 */
- (NSUInteger)getSize;

/** 获取磁盘缓存中的图像数量。
 * Get the number of images in the disk cache
 */
- (NSUInteger)getDiskCount;

/** 异步计算磁盘缓存的大小。
 * Asynchronously calculate the disk cache's size.
 */
- (void)calculateSizeWithCompletionBlock:(nullable SDWebImageCalculateSizeBlock)completionBlock;

#pragma mark - Cache Paths

/** 获取某个确定key(密钥)的缓存路径（需要缓存路径根文件夹）
 *  Get the cache path for a certain key (needs the cache path root folder)
 *  用正在使用的url获得
 *  @param key  the key (can be obtained from url using cacheKeyForURL)
 *  @param path the cache path root folder
 *
 *  @return the cache path
 */
- (nullable NSString *)cachePathForKey:(nullable NSString *)key inPath:(nonnull NSString *)path;

/** 获取某个键的默认缓存路径
 *  Get the default cache path for a certain key
 *  获得
 *  @param key the key (can be obtained from url using cacheKeyForURL)
 *
 *  @return the default cache path
 */
- (nullable NSString *)defaultCachePathForKey:(nullable NSString *)key;

@end

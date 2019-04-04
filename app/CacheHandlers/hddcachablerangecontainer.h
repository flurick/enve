#ifndef HDDCACHABLERANGECONTAINER_H
#define HDDCACHABLERANGECONTAINER_H
#include "rangecachecontainer.h"
#include "minimalcachehandler.h"
#include "tmpfilehandlers.h"
class _ScheduledTask;

template <typename T>
class HDDCachableRangeContainer :
        public RangeCacheContainer {
protected:
    typedef MinimalCacheHandler<T> Handler;
    HDDCachableRangeContainer() :
        mParentCacheHandler_k(nullptr) {}
    HDDCachableRangeContainer(const FrameRange &range, Handler * const parent) :
        RangeCacheContainer(range), mParentCacheHandler_k(parent) {}
    virtual stdsptr<_HDDTask> createTmpFileDataSaver() = 0;
    virtual stdsptr<_HDDTask> createTmpFileDataLoader() = 0;
    virtual void clearDataAfterSaved() = 0;
public:
    ~HDDCachableRangeContainer() {
        scheduleDeleteTmpFile();
    }

    _ScheduledTask* scheduleLoadFromTmpFile() {
        if(mSavingToFile) {
            mCancelAfterSaveDataClear = true;
            return mSavingUpdatable.get();
        }
        if(!mNoDataInMemory) return nullptr;
        if(mLoadingFromFile) return mLoadingUpdatable.get();

        mLoadingFromFile = true;
        mLoadingUpdatable = createTmpFileDataLoader();
        mLoadingUpdatable->scheduleTask();
        return mLoadingUpdatable.get();
    }

    bool cacheAndFree() {
        if(mBlocked || mNoDataInMemory || mSavingToFile) return false;
        if(!mParentCacheHandler_k) return false;
        saveToTmpFile();
        return true;
    }

    void setBlocked(const bool &bT) {
        if(bT == mBlocked) return;
        mBlocked = bT;
        if(bT) {
            MemoryHandler::sGetInstance()->removeContainer(this);
            scheduleLoadFromTmpFile();
        } else {
            MemoryHandler::sGetInstance()->addContainer(this);
        }
    }

    bool freeAndRemove() {
        if(mBlocked || mSavingToFile) return false;
        if(!mParentCacheHandler_k) return false;
        mParentCacheHandler_k->removeRenderContainer(ref<T>());
        return true;
    }

    void scheduleDeleteTmpFile() {
        if(!mTmpFile) return;
        stdsptr<_ScheduledTask> updatable =
                SPtrCreate(CacheContainerTmpFileDataDeleter)(mTmpFile);
        mTmpFile.reset();
        updatable->scheduleTask();
    }

    void saveToTmpFile() {
        if(mSavingToFile || mNoDataInMemory) return;
        mMemSizeAwaitingSave = getByteCount();
        MemoryHandler::sGetInstance()->incMemoryScheduledToRemove(mMemSizeAwaitingSave);
        MemoryHandler::sGetInstance()->removeContainer(this);
        mSavingToFile = true;
        mNoDataInMemory = true;
        mCancelAfterSaveDataClear = false;
        mSavingUpdatable = createTmpFileDataSaver();
        mSavingUpdatable->scheduleTask();
    }

    void setDataSavedToTmpFile(const qsptr<QTemporaryFile> &tmpFile) {
        mSavingUpdatable.reset();
        mTmpFile = tmpFile;
        mSavingToFile = false;
        if(mCancelAfterSaveDataClear) {
            mNoDataInMemory = false;
            mCancelAfterSaveDataClear = false;
            if(!mBlocked) {
                MemoryHandler::sGetInstance()->addContainer(this);
            }
            return;
        } else {
            mNoDataInMemory = true;
        }
        clearDataAfterSaved();
        MemoryHandler::sGetInstance()->incMemoryScheduledToRemove(
                    -mMemSizeAwaitingSave);
    }

    bool storesDataInMemory() const {
        return !mNoDataInMemory;
    }
protected:
    void afterDataLoadedFromTmpFile() {
        mLoadingFromFile = false;
        mNoDataInMemory = false;
        mLoadingUpdatable.reset();
        if(!mBlocked) {
            MemoryHandler::sGetInstance()->addContainer(this);
        }
    }

    void afterDataReplaced() {
        if(mNoDataInMemory) {
            mNoDataInMemory = false;
            scheduleDeleteTmpFile();
        }
    }

    stdsptr<_ScheduledTask> mLoadingUpdatable;
    stdsptr<_ScheduledTask> mSavingUpdatable;

    qsptr<QTemporaryFile> mTmpFile;

    bool mCancelAfterSaveDataClear = false;
    bool mSavingToFile = false;
    bool mLoadingFromFile = false;
    bool mNoDataInMemory = false;

    int mMemSizeAwaitingSave = 0;
    Handler * const mParentCacheHandler_k;
};

#endif // HDDCACHABLERANGECONTAINER_H

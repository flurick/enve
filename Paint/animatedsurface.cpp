#include "animatedsurface.h"
#include "Boxes/paintbox.h"

void getTileDrawers(const std::shared_ptr<TilesData> &drawTilesData,
                    const int &drawTilesFrame,
                    const int &neighDrawerRelFrame,
                    const int &currRelFrame,
                    const int &additionalFrames,
                    QList<TileSkDrawerCollection> *tileDrawers) {
    int alpha;
    if(neighDrawerRelFrame == INT_MIN ||
       neighDrawerRelFrame == INT_MAX) {
        alpha = 255;
    } else {
        int dR = qAbs(drawTilesFrame - neighDrawerRelFrame) + additionalFrames;
        int dFrame = qAbs(drawTilesFrame - currRelFrame);
        qreal alphaT = dFrame*1./dR;
        alpha = qMin(255, qMax(0, qRound((1. - alphaT/*alphaT*/)*255.)) );
    }
    TileSkDrawerCollection coll;
    coll.alpha = static_cast<uchar>(alpha);
    drawTilesData->getTileDrawers(&coll.drawers);
    tileDrawers->append(coll);
}

AnimatedSurface::AnimatedSurface(const ushort &widthT,
                                 const ushort &heightT,
                                 const qreal &scale,
                                 PaintBox *parentBox) :
    Surface(widthT, heightT, scale, true),
    Animator("canvas") {
    mParentBox = parentBox;
}

AnimatedSurface::~AnimatedSurface() {}

void AnimatedSurface::currentDataModified() {
    if(prp_hasKeys()) {
        if(prp_isKeyOnCurrentFrame()) {
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            Key *key = anim_getPrevKey(anim_mCurrentRelFrame);
            if(key == nullptr) {
                key = anim_getNextKey(anim_mCurrentRelFrame);
            }
            anim_updateAfterChangedKey(key);
        }
    } else {
        prp_updateInfluenceRangeAfterChanged();
    }
}

void AnimatedSurface::anim_saveCurrentValueAsKey() {
    if(prp_isKeyOnCurrentFrame()) return;
    newEmptyPaintFrame();
    SurfaceKey *key = getAsPtr(anim_mKeyOnCurrentFrame, SurfaceKey);
    SurfaceKey *prevKey = getAsPtr(anim_getPrevKey(key), SurfaceKey);
    TilesData *tiles;
    if(prevKey == nullptr) {
        tiles = mCurrentTiles.get();
    } else {
        tiles = prevKey->getTilesData();
    }
    key->duplicateTilesContentFrom(tiles);
}

void AnimatedSurface::newEmptyPaintFrame(const int &relFrame) {
    if(!anim_mIsRecording) {
        anim_setRecordingValue(true);
    }
    Key *keyAtFrame = anim_getKeyAtRelFrame(relFrame);
    if(keyAtFrame != nullptr) {
        getAsPtr(keyAtFrame, SurfaceKey)->getTilesData()->clearTiles();
        return;
    }

    SurfaceKey *prevKey = getAsPtr(anim_getPrevKey(relFrame), SurfaceKey);
    if(prevKey != nullptr) {
        if(prevKey->getRelFrame() == relFrame) {
            prevKey->getTilesData()->clearTiles();
            return;
        }
    }

    SurfaceKeySPtr frameT = SPtrCreate(SurfaceKey)(this);
    frameT->setRelFrame(relFrame);
    if(prp_hasKeys()) {
        frameT->setSize(static_cast<ushort>(mWidth),
                        static_cast<ushort>(mHeight));
    } else {
        frameT->setTiles(mCurrentTiles);
    }

    anim_appendKey(frameT);

    updateTargetTiles();
}

void AnimatedSurface::newEmptyPaintFrame() {
    newEmptyPaintFrame(anim_mCurrentRelFrame);
}

void AnimatedSurface::updateTargetTiles() {
    mCurrentTiles->setCurrentlyUsed(false);
    foreach(const std::shared_ptr<TilesData> &tilesDataT, mDrawTilesData) {
        tilesDataT->setCurrentlyUsed(false);
    }
    mDrawTilesData.clear();
    mDrawTilesFrames.clear();
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId,
                                               anim_mCurrentRelFrame)) {
        SurfaceKey *prevKey = getAsPtr(anim_mKeys.at(prevId), SurfaceKey);
        SurfaceKey *nextKey = getAsPtr(anim_mKeys.at(nextId), SurfaceKey);

        TilesData *prevKeyTilesData = prevKey->getTilesData();
        prevKeyTilesData->setCurrentlyUsed(true);
        mDrawTilesData.append(getAsSPtr(prevKeyTilesData, TilesData));
        mDrawTilesFrames.append(prevKey->getRelFrame());
        if(mIsDraft) {
            int idT = prevId - 1;
            while(idT >= 0) {
                SurfaceKey *keyT = getAsPtr(anim_mKeys.at(idT), SurfaceKey);
                TilesData *keyTTilesData = keyT->getTilesData();
                keyTTilesData->setCurrentlyUsed(true);
                mDrawTilesData.prepend(getAsSPtr(keyTTilesData, TilesData));
                mDrawTilesFrames.prepend(keyT->getRelFrame());
                idT--;
                if(anim_mCurrentRelFrame - keyT->getRelFrame() >=
                        mOverlapFrames) break;
            }
            if(nextKey != prevKey) {
                TilesData *nextKeyTilesData = nextKey->getTilesData();
                nextKeyTilesData->setCurrentlyUsed(true);
                mDrawTilesData.append(getAsSPtr(nextKeyTilesData, TilesData));
                mDrawTilesFrames.append(nextKey->getRelFrame());
            }
            idT = nextId + 1;
            while(idT < anim_mKeys.count()) {
                SurfaceKey *keyT = getAsPtr(anim_mKeys.at(idT), SurfaceKey);
                TilesData *keyTTilesData = keyT->getTilesData();
                keyTTilesData->setCurrentlyUsed(true);
                mDrawTilesData.append(getAsSPtr(keyTTilesData, TilesData));
                mDrawTilesFrames.append(keyT->getRelFrame());
                idT++;
                if(keyT->getRelFrame() - anim_mCurrentRelFrame >=
                        mOverlapFrames) break;
            }
        }

        int nextDFrame = qAbs(nextKey->getRelFrame() - anim_mCurrentRelFrame);
        int prevDFrame = qAbs(prevKey->getRelFrame() - anim_mCurrentRelFrame);

        if(nextDFrame > prevDFrame) {
            mCurrentTiles = getAsSPtr(prevKey->getTilesData(), TilesData);
        } else {
            mCurrentTiles = getAsSPtr(nextKey->getTilesData(), TilesData);
        }
    }
    mCurrentTiles->setCurrentlyUsed(true);
}

void AnimatedSurface::setCurrentRelFrame(const int &relFrame) {
    anim_mCurrentRelFrame = relFrame;
    updateTargetTiles();
}

void AnimatedSurface::getTileDrawers(QList<TileSkDrawerCollection> *tileDrawers) {
    if(mDrawTilesFrames.isEmpty()) {
        TileSkDrawerCollection coll;
        mCurrentTiles->getTileDrawers(&coll.drawers);
        tileDrawers->append(coll);
    } else {
        int countT = mDrawTilesFrames.count();
        int prevRelFrame = INT_MIN;
        for(int i = 0; i < countT; i++) {
            const int &relFrame = mDrawTilesFrames.at(i);
            std::shared_ptr<TilesData> tilesData = mDrawTilesData.at(i);
            int neighRelFrame;
            if(relFrame < anim_mCurrentRelFrame) {
                if(i + 1 < countT) {
                    neighRelFrame = mDrawTilesFrames.at(i + 1);
                } else {
                    neighRelFrame = INT_MAX;
                }

            } else {
                neighRelFrame = prevRelFrame;
            }
//            ::getTileDrawers(tilesData, relFrame,
//                             neighRelFrame, anim_mCurrentRelFrame,
//                             mOverlapFrames,
//                             tileDrawers);

            TileSkDrawerCollection coll;
            tilesData->getTileDrawers(&coll.drawers);
            if(mIsDraft) {
                int alpha;
                if(neighRelFrame == INT_MIN ||
                   neighRelFrame == INT_MAX) {
                    alpha = 255;
                } else {
    //                int dR = qAbs(relFrame - neighRelFrame) + mOverlapFrames;
    //                int dFrame = qAbs(relFrame - anim_mCurrentRelFrame);
    //                qreal alphaT = dFrame*1./dR;
    //                alpha = qMin(255, qMax(0, qRound((1. - alphaT/*alphaT*/)*255.)) );
                    qreal alphaT = 0.;
                    if((relFrame < neighRelFrame &&
                        anim_mCurrentRelFrame >= neighRelFrame) ||
                        (relFrame > neighRelFrame &&
                         anim_mCurrentRelFrame <= neighRelFrame)) {
                        alphaT = (qAbs(anim_mCurrentRelFrame - neighRelFrame) + 1)*1./(mOverlapFrames + 1);
                    }
                    alpha = qMin(255, qMax(0, qRound((1. - alphaT/*alphaT*/)*255.)) );
                }
                coll.alpha = static_cast<uchar>(alpha);
                bool hueChange = relFrame != anim_mCurrentRelFrame;
                coll.setHueChangeEnabled(hueChange);

                if(hueChange) {
                    coll.setHue(relFrame > anim_mCurrentRelFrame ? 0. : 0.5);
                }
            }

            tileDrawers->append(coll);

            prevRelFrame = relFrame;
        }
    }
}

void AnimatedSurface::anim_removeKey(const KeySPtr& keyToRemove,
                                     const bool &saveUndoRedo) {
    Animator::anim_removeKey(keyToRemove, saveUndoRedo);
    updateTargetTiles();
}

void AnimatedSurface::anim_appendKey(
                    const KeySPtr& newKey,
                    const bool &saveUndoRedo,
                    const bool &update) {
    Animator::anim_appendKey(newKey, saveUndoRedo, update);
    updateTargetTiles();
}

void AnimatedSurface::anim_moveKeyToRelFrame(Key *key,
                            const int &newFrame,
                            const bool &saveUndoRedo,
                            const bool &finish) {
    Animator::anim_moveKeyToRelFrame(key, newFrame,
                                     saveUndoRedo, finish);
    updateTargetTiles();
}

void AnimatedSurface::setSize(const ushort &width_t,
                              const ushort &height_t) {
    // initialize tiles
    if(width_t == mWidth && height_t == mHeight) return;
    if(prp_hasKeys()) {
        int n_tile_cols_t = qCeil(width_t/static_cast<qreal>(TILE_DIM));
        int n_tile_rows_t = qCeil(height_t/static_cast<qreal>(TILE_DIM));
        Q_FOREACH(const KeySPtr &key, anim_mKeys) {
            SurfaceKey *frameT = getAsPtr(key, SurfaceKey);
            frameT->setSize(width_t,
                            height_t);
        }
        mWidth = width_t;
        mHeight = height_t;
        mNTileRows = n_tile_rows_t;
        mNTileCols = n_tile_cols_t;
    } else {
        Surface::setSize(width_t, height_t);
    }
}

void AnimatedSurface::move(const int &xT, const int &yT) {
    if(prp_hasKeys()) {
        Q_FOREACH(const KeySPtr &key, anim_mKeys) {
            SurfaceKey *frameT = getAsPtr(key, SurfaceKey);
            frameT->getTilesData()->move(xT, yT);
        }
    } else {
        Surface::move(xT, yT);
    }
}

void AnimatedSurface::tabletPressEvent(const qreal &xT, const qreal &yT,
                                       const ulong &time_stamp,
                                       const qreal &pressure,
                                       const bool &erase,
                                       const Brush *brush) {
    if(prp_isRecording() && !prp_isKeyOnCurrentFrame()) {
        newEmptyPaintFrame();
    }
    Surface::tabletPressEvent(xT, yT, time_stamp,
                              pressure, erase, brush);
}

bool AnimatedSurface::prp_differencesBetweenRelFrames(const int &relFrame1,
                                                      const int &relFrame2) {
    if(anim_mKeys.count() > 1) {
        int firstKeyRelFrame = anim_mKeys.first()->getRelFrame();
        int lastKeyRelFrame = anim_mKeys.last()->getRelFrame();
        if(relFrame1 >= lastKeyRelFrame + mOverlapFrames &&
           relFrame2 >= lastKeyRelFrame + mOverlapFrames) return false;
        if(relFrame1 <= firstKeyRelFrame - mOverlapFrames &&
           relFrame2 <= firstKeyRelFrame - mOverlapFrames) return false;
        return true;
    }
    return false;
}

void AnimatedSurface::anim_updateAfterChangedKey(Key *key) {
    if(anim_mIsComplexAnimator) return;
    if(key == nullptr) {
        prp_updateInfluenceRangeAfterChanged();
        return;
    }
    int prevKeyRelFrame = anim_getPrevKeyRelFrame(key);
    if(prevKeyRelFrame != INT_MIN) {
        prevKeyRelFrame++;
        prevKeyRelFrame -= mOverlapFrames;
    }
    int nextKeyRelFrame = anim_getNextKeyRelFrame(key);
    if(nextKeyRelFrame != INT_MAX) {
        nextKeyRelFrame--;
        nextKeyRelFrame += mOverlapFrames;
    }

    prp_updateAfterChangedRelFrameRange(prevKeyRelFrame,
                                        nextKeyRelFrame);
}

void AnimatedSurface::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                           int *lastIdentical,
                                                           const int &relFrame) {
    Animator::prp_getFirstAndLastIdenticalRelFrame(
                                firstIdentical,
                                lastIdentical,
                                relFrame);
    if(!mIsDraft) return;
    if(anim_mKeys.count() > 1) {
//        if(anim_mKeys.count() == 1) {
//            int keyFrame = anim_mKeys.first()->getRelFrame();
//            if(relFrame > keyFrame - mAdditionalFrames &&
//               relFrame < keyFrame + mAdditionalFrames) {
//                *firstIdentical = relFrame;
//                *lastIdentical = relFrame;
//            } else {
//                if(relFrame > keyFrame) {
//                    *firstIdentical = keyFrame + mAdditionalFrames;
//                } else {
//                    *lastIdentical = keyFrame - mAdditionalFrames;
//                }
//            }
//            return;
//        }
        if(*firstIdentical == INT_MIN) {
            int firstKeyFrame = anim_mKeys.first()->getRelFrame();
            if(firstKeyFrame - mOverlapFrames < relFrame) {
                *firstIdentical = relFrame;
                *lastIdentical = relFrame;
                return;
            } else {
                *lastIdentical = firstKeyFrame - mOverlapFrames;
            }
        } else if(*lastIdentical == INT_MAX) {
            int lastKeyFrame = anim_mKeys.last()->getRelFrame();
            if(lastKeyFrame + mOverlapFrames > relFrame) {
                *firstIdentical = relFrame;
                *lastIdentical = relFrame;
            } else {
                *firstIdentical = lastKeyFrame + mOverlapFrames;
            }
        }
    }
}

SurfaceKey::SurfaceKey(Animator *parentAnimator) :
    Key(parentAnimator){
}

SurfaceKey::~SurfaceKey() {
}
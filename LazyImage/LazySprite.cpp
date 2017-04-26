/****************************************************************************
 Copyright (c) 2015 QuanNguyen
 
 http://quannguyen.info
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "LazySprite.h"

#include "LazyImageLoader.h"

USING_NS_CC;

LazySprite::LazySprite()
: _holderSprite(nullptr)
, _loadImageListener(nullptr)
{
    
}

LazySprite::~LazySprite()
{
    CC_SAFE_RELEASE_NULL(_holderSprite);
}

LazySprite* LazySprite::create(cocos2d::Sprite *holder, const cocos2d::Size& s)
{
    LazySprite *l = new LazySprite();
    if(l && l->init(holder, s)){
        l->autorelease();
        return l;
    }
    
    CC_SAFE_DELETE(l);
    return nullptr;
}

bool LazySprite::init(Sprite *holder, const cocos2d::Size& s)
{
    CCASSERT(holder, "Holder must be not null");
    
    if(!Sprite::initWithSpriteFrame(holder->getSpriteFrame())){
        return false;
    }
    
    _imageSize = s;
    
    resetScaleBySize(getContentSize());
    
    _holderSprite = holder;
    CC_SAFE_RETAIN(_holderSprite);
    
    return true;
}

void LazySprite::setImageURL(const std::string &url)
{
    CCLOG("LazySprite::setImageURL: %s", url.c_str());
    if(url == _imgURL){
        return;
    }
    
    _imgURL = url;
    
    std::string path = LazyImageLoader::getInstance()->pathForLoadedImage(url);
    if(path.length() != 0){
        Sprite *s = Sprite::create(path);
        setSpriteFrame(s->getSpriteFrame());
        resetScaleBySize(s->getBoundingBox().size);
        return;
    }
    
    //reset to holder
    setSpriteFrame(_holderSprite->getSpriteFrame());
    resetScaleBySize(_holderSprite->getContentSize());
    
    //request load image
    LazyImageLoader::getInstance()->loadImage(url);
}

void LazySprite::reset()
{
    setSpriteFrame(_holderSprite->getSpriteFrame());
    resetScaleBySize(_holderSprite->getContentSize());
    _imgURL = "";
}

void LazySprite::onLoadSpriteDone(cocos2d::Event *obj)
{
    CCLOG("LazySprite: recieve notification onload sprite done");
    
    ImageLoaderEvent *eventCustom = dynamic_cast<ImageLoaderEvent *> (obj);
    if(!eventCustom){
        return;
    }
    std::string url = eventCustom->getURL();
    if(url != _imgURL){
        return;
    }
    //update sprite
    Texture2D *tex = eventCustom->getTexture();
    
    if(tex != NULL){
        Sprite *s = Sprite::createWithTexture(tex);
        //update to sprite
        setSpriteFrame(s->getSpriteFrame());
        resetScaleBySize(s->getBoundingBox().size);
    }
}

void LazySprite::resetScaleBySize(cocos2d::Size s)
{
    if(_imageSize.width != 0 && _imageSize.height != 0){
        //scale
        setScaleX(_imageSize.width/s.width);
        setScaleY(_imageSize.height/s.height);
    }
}

void LazySprite::onEnter()
{
    Sprite::onEnter();
    
    //register notification
    EventListenerCustom *listener = EventListenerCustom::create(EVENT_LAZY_IMAGE_DONE, CC_CALLBACK_1(LazySprite::onLoadSpriteDone, this));
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
    _loadImageListener = listener;

}

void LazySprite::onEnterTransitionDidFinish()
{
    Sprite::onEnterTransitionDidFinish();
    
    if(_imgURL.size() != 0){
        std::string path = LazyImageLoader::getInstance()->pathForLoadedImage(_imgURL);
        if(path.size() != 0){
            Sprite *s = Sprite::create(path);
            //update to sprite
            setSpriteFrame(s->getSpriteFrame());
            resetScaleBySize(s->getBoundingBox().size);
        }
    }
}

void LazySprite::onExit()
{
    if(_loadImageListener){
        getEventDispatcher()->removeEventListener(_loadImageListener);
        _loadImageListener = nullptr;
    }
    
    Sprite::onExit();
}

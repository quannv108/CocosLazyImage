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

#ifndef __Funny__LazySprite__
#define __Funny__LazySprite__

#include "cocos2d.h"

class LazySprite : public cocos2d::Sprite {
protected:
    LazySprite();
    virtual ~LazySprite();
    
public:
    static LazySprite* create(cocos2d::Sprite *holder, const cocos2d::Size& s);
    void setImageURL(const std::string& url);
    void reset();
protected:
    virtual bool init(cocos2d::Sprite *holder, const cocos2d::Size& s);
    virtual void onEnter() override;
    virtual void onEnterTransitionDidFinish() override;
    virtual void onExit() override;
    
    CC_SYNTHESIZE_READONLY_PASS_BY_REF(cocos2d::Size, _imageSize, ImageSize);
    CC_SYNTHESIZE_READONLY_PASS_BY_REF(std::string, _imgURL, ImageURL);
    
private:
    void onLoadSpriteDone(cocos2d::Event *obj);
    void resetScaleBySize(cocos2d::Size s);
    
private:
    cocos2d::Sprite *_holderSprite;
    cocos2d::EventListenerCustom *_loadImageListener;
};

#endif /* defined(__Funny__LazySprite__) */

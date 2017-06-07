/****************************************************************************
 Copyright (c) 2016 QuanNguyen
 
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

#include "LazyImageLoader.h"

#include <algorithm>

#define kCacheDir   "LazyImageCache/"
#define kCacheFile   "imageCacheInfo.txt"

USING_NS_CC;

LazyImageLoader::LazyImageLoader()
: _useOwnFolder(false)
, _downloader(NULL)
{
    
}

LazyImageLoader::~LazyImageLoader()
{
    CC_SAFE_DELETE(_downloader);
    _downloader = NULL;
}

static LazyImageLoader* _sharedInstance = NULL;

LazyImageLoader* LazyImageLoader::getInstance()
{
    if(_sharedInstance == NULL){
        _sharedInstance = new LazyImageLoader();
        _sharedInstance->init();
    }
    return _sharedInstance;
}

bool LazyImageLoader::init()
{
    _writablePath = FileUtils::getInstance()->getWritablePath();
    
    std::string ownDirPath =  _writablePath + std::string(kCacheDir);
    
    if (!FileUtils::getInstance()->createDirectory(ownDirPath.c_str()))
    {
        CCLOG("LazyImageLoader: can not create directory %s", ownDirPath.c_str());
        _useOwnFolder = false;
        _writePathPrefix = "";
    }else{
        _useOwnFolder = true;
        _writePathPrefix = std::string(kCacheDir);
    }
    
    network::DownloaderHints hints;
    hints.countOfMaxProcessingTasks = 10;
    hints.timeoutInSeconds = 10;
    hints.tempFileNameSuffix = "lazyimageloader";
    
    _downloader = new cocos2d::network::Downloader(hints);
    
    _downloader->onFileTaskSuccess = CC_CALLBACK_1(LazyImageLoader::onDownloadTaskDone, this);
    _downloader->onTaskError = std::bind(&LazyImageLoader::onDownloadTaskFailed, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    
    
    fileList = FileUtils::getInstance()->getValueMapFromFile(_writablePath + _writePathPrefix + kCacheFile);
    
    deleteExpiredImages();
    
    return true;
}

#pragma mark - utils

void LazyImageLoader::deleteExpiredImages()
{
    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    cocos2d::ValueMap tempMap;
    
    for (auto& kv : fileList) {
        
        double cacheDuration = kv.second.asDouble();
        if(cacheDuration > -1 &&  cacheDuration < ((double)currentTime))
        {
            FileUtils::getInstance()->removeFile(_writablePath + _writePathPrefix + convertURLToFilePath(kv.first));
        }
        else
        {
            tempMap[kv.first] = kv.second;
        }
    }
    
    fileList = tempMap;
    FileUtils::getInstance()->writeValueMapToFile(fileList, _writablePath + _writePathPrefix + kCacheFile);
}


std::string LazyImageLoader::pathForLoadedImage(const std::string &url)
{
    
    //check already have
    std::string filePath = convertURLToFilePath(url);
    if(filePath.size() == 0){
        return "";
    }
    
    std::string fullPath = _writablePath + _writePathPrefix + filePath;
    if(FileUtils::getInstance()->isFileExist(fullPath)){
        return fullPath;
    }
    
    return "";
}

void LazyImageLoader::createDirectoryForPath(const std::string& path)
{
    std::vector<std::string> subPart = split(path, '/');
    if(subPart.size() == 0){
        return;
    }
    
    //create diretory if dont have
    std::string corePath = _writablePath + _writePathPrefix;
    for (int i = 0; i < subPart.size() - 1; i ++) {
        corePath.append(subPart.at(i));
        if(!FileUtils::getInstance()->isDirectoryExist(corePath)){
            FileUtils::getInstance()->createDirectory(corePath);
        }
        corePath.append("/");
    }
}

std::string LazyImageLoader::convertURLToFilePath(const std::string &url)
{
    if(url.length() == 0){
        return "";
    }
    
    std::string output(url);
    //remove https:// & http:// & www
    replace(output, "https://", "");
    replace(output, "http://", "");
    replace(output, "www.", "");
    replace(output, "blob:", "");
    
    //change ?, &
    std::replace(output.begin(), output.end(), '?', '_');
    std::replace(output.begin(), output.end(), '&', '-');
    
    if(output.size() == 0){
        return "";
    }
    
    if(output.at(output.size() - 1) == '/'){
        output.erase(output.begin() + output.size() - 1);
    }
    
    std::vector<std::string> subPart = split(output, '/');
    if(subPart.size() == 0){
        return "";
    }
    
    if(subPart.size() == 1){
//        CCLOG("%s from %s to %s", __PRETTY_FUNCTION__, url.c_str(), output.c_str());
        return output;
    }
    
    std::string finalPart = subPart.at(subPart.size() - 1);
    
    //add png if dont have extension
    bool haveExtension = false;
    std::vector<std::string> spl = split(finalPart, '.');
    if(spl.size() > 1){
        std::string ext = spl.at(spl.size() - 1);
        if(ext.length() == 0){
            //should remove last character
            output.erase(output.begin() + output.size() - 1);
        }else{
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if(ext == "png" || ext == "jpg" || ext == "jpeg"
               || ext == "gif" || ext == "webp")
            {
                haveExtension = true;
            }
        }
    }
    
    if(!haveExtension){
        output.append(".png");
    }
    
//    CCLOG("%s from %s to %s", __PRETTY_FUNCTION__, url.c_str(), output.c_str());
    return output;
}

bool LazyImageLoader::replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos){
        return false;
    }
    str.replace(start_pos, from.length(), to);
    return true;
}

std::vector<std::string> LazyImageLoader::split(const std::string& str, char delimiter)
{
    std::vector<std::string> internal;
    std::stringstream ss(str); // Turn the string into a stream.
    std::string tok;
    while(getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }
    
    return internal;
}

#pragma mark - downloader

void LazyImageLoader::saveCacheInfo(const std::string &url,double cacheDuration)
{
    auto currentTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    fileList[url] = cacheDuration >= 0 ? (double)currentTime + cacheDuration : -1;
    FileUtils::getInstance()->writeValueMapToFile(fileList, _writablePath + _writePathPrefix + kCacheFile);
    CCLOG("LazyImageLoader:: cache %s done for %f seconds", url.c_str(), cacheDuration);
}

bool LazyImageLoader::loadImage(const std::string &url,double cacheDuration)
{
    
    std::string filePath = convertURLToFilePath(url);
    if(filePath.size() == 0){
        return false;
    }
    
    //check already have
    std::string fullPath = _writablePath + _writePathPrefix + filePath;
    if(FileUtils::getInstance()->isFileExist(fullPath)){
        
        CCLOG("%s: %s already loaded, skip", __PRETTY_FUNCTION__, url.c_str());
        return false;
    }
    
    std::string iden = fullPath;
    ImageLoadInfo loadInfo = {fullPath,cacheDuration};
    
    
    auto ite = std::find_if(_loadersIdentifier.begin(), _loadersIdentifier.end(), [loadInfo](const ImageLoadInfo& m) -> bool {
        return loadInfo.url.compare(m.url) == 0;
    });
    if(ite != _loadersIdentifier.end()){
        //downloading...skip
        return false;
    }
    
    CCLOG("%s will load image %s to path %s", __PRETTY_FUNCTION__, url.c_str(), fullPath.c_str());
    
    createDirectoryForPath(filePath);
    
    _loadersIdentifier.push_back(loadInfo);
    
    _downloader->createDownloadFileTask(url, fullPath, iden);
    
    return true;
}

void LazyImageLoader::onDownloadTaskDone(const cocos2d::network::DownloadTask &task)
{
    Image* img = new Image();
    if(!img->initWithImageFile(task.storagePath)){
        //init file failed
        CC_SAFE_DELETE(img);
        CCLOG("LazyImageLoader:: load %s done but no image", task.requestURL.c_str());
        return;
    }
    
    cocos2d::Texture2D* texture = new Texture2D() ;
    texture->initWithImage(img);
    
    texture->autorelease();
    img->release();
    
    //old style
    if (texture->getContentSize().width == 0 ||
        texture->getContentSize().height == 0) {
        CCLOG("LazyImageLoader:: load %s done but no image", task.requestURL.c_str());
        return;
    }
    
    CCLOG("LazyImageLoader:: load %s done to %s", task.requestURL.c_str(), task.storagePath.c_str());
    this->reportLoadDone(task.requestURL, texture);
    
    //remove out of queue
    do{
        auto identifier = task.identifier;
        
        auto ite = std::find_if(_loadersIdentifier.begin(), _loadersIdentifier.end(), [identifier](const ImageLoadInfo& m) -> bool {
            return identifier.compare(m.url) == 0;
        });
        
        if(ite != _loadersIdentifier.end()){
            
            //save cache info
            saveCacheInfo(task.requestURL,ite->cacheDuration);
            
            _loadersIdentifier.erase(ite);
        }else{
            break;
        }
    }while(true);
}

void LazyImageLoader::onDownloadTaskFailed(const cocos2d::network::DownloadTask &task,
                                           int errorCode,
                                           int errorCodeInternal,
                                           const std::string &errorStr)
{
    CCLOG("LazyImageLoader:: load %s failed: %d %d %s",
          task.requestURL.c_str(), errorCode, errorCodeInternal, errorStr.c_str());
    
    //remove out of queue
    
    auto identifier = task.identifier;
    
    auto ite = std::find_if(_loadersIdentifier.begin(), _loadersIdentifier.end(), [identifier](const ImageLoadInfo& m) -> bool {
        return identifier.compare(m.url)==0;
    });
    
    if(ite != _loadersIdentifier.end()){
        _loadersIdentifier.erase(ite);
    }
    
    //retry
    if(errorCode == -3 && errorCodeInternal == -1001){
        //request time out
        loadImage(task.requestURL);
    }
}

#pragma mark - report

void LazyImageLoader::reportLoadDone(const std::string &url, cocos2d::Texture2D *tex)
{
    CCLOG("LazyImageLoader::reportLoadDone: %s", url.c_str());
    ImageLoaderEvent *event = new ImageLoaderEvent();
    event->setURL(url);
    event->setTexture(tex);
    Director::getInstance()->getEventDispatcher()->dispatchEvent(event);
    event->release();
}

#include <easy2d/GLTextureAtlas.h>
#include <easy2d/e2dbase.h>
#include <algorithm>

namespace easy2d
{

// GLTextureAtlasPage 实现

GLTextureAtlasPage::GLTextureAtlasPage(int width, int height)
    : _width(width)
    , _height(height)
    , _textureID(0)
    , _texture(nullptr)
    , _gridCellSize(16)
{
    _gridWidth = (_width + _gridCellSize - 1) / _gridCellSize;
    _gridHeight = (_height + _gridCellSize - 1) / _gridCellSize;
    _usedGrid.resize(_gridWidth * _gridHeight, false);
}

GLTextureAtlasPage::~GLTextureAtlasPage()
{
    if (_texture)
    {
        delete _texture;
        _texture = nullptr;
    }
}

bool GLTextureAtlasPage::initialize()
{
    // 创建纹理对象
    _texture = new GLTexture();
    
    // 创建空白纹理数据
    std::vector<unsigned char> emptyData(_width * _height * 4, 0);
    
    if (!_texture->createFromRawData(_width, _height, emptyData.data(), false))
    {
        E2D_ERROR("Failed to create texture atlas page");
        delete _texture;
        _texture = nullptr;
        return false;
    }
    
    _textureID = _texture->getID();
    
    // 设置纹理参数
    _texture->setFilterMode(GL_LINEAR, GL_LINEAR);
    _texture->setWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    
    E2D_LOG("Created texture atlas page: %dx%d", _width, _height);
    return true;
}

bool GLTextureAtlasPage::packTexture(GLTexture* texture, AtlasRegion& outRegion)
{
    if (!texture || !texture->isValid())
    {
        return false;
    }
    
    int texWidth = texture->getWidth();
    int texHeight = texture->getHeight();
    
    // 查找空闲区域
    Rect freeRect;
    if (!findFreeRegion(texWidth, texHeight, freeRect))
    {
        return false;
    }
    
    // 复制纹理数据
    copyTextureData(texture, freeRect);
    
    // 填充区域信息
    outRegion.textureID = texture->getID();
    outRegion.region = freeRect;
    outRegion.uv0 = Point(freeRect.getLeft() / _width, freeRect.getTop() / _height);
    outRegion.uv1 = Point(freeRect.getRight() / _width, freeRect.getBottom() / _height);
    
    // 标记区域为已使用
    markRegionUsed(freeRect);
    _occupiedRegions.push_back(freeRect);
    
    return true;
}

float GLTextureAtlasPage::getUsageRatio() const
{
    if (_occupiedRegions.empty())
    {
        return 0.0f;
    }
    
    float usedArea = 0.0f;
    for (const auto& rect : _occupiedRegions)
    {
        usedArea += rect.getWidth() * rect.getHeight();
    }
    
    float totalArea = static_cast<float>(_width) * static_cast<float>(_height);
    return usedArea / totalArea;
}

void GLTextureAtlasPage::bind(int slot) const
{
    if (_texture)
    {
        _texture->bind(slot);
    }
}

bool GLTextureAtlasPage::findFreeRegion(int width, int height, Rect& outRect)
{
    // 使用简单的 Shelf 算法
    // 从左到右、从上到下查找合适位置
    
    int padding = 2; // 纹理间填充
    int requiredWidth = width + padding;
    int requiredHeight = height + padding;
    
    // 如果纹理太大，直接失败
    if (requiredWidth > _width || requiredHeight > _height)
    {
        return false;
    }
    
    // 简单的贪心算法：找第一个能放下的位置
    // 按Y坐标遍历每一行
    for (int y = 0; y <= _height - requiredHeight; y += _gridCellSize)
    {
        for (int x = 0; x <= _width - requiredWidth; x += _gridCellSize)
        {
            Rect testRect(Point(static_cast<float>(x), static_cast<float>(y)), 
                         Size(static_cast<float>(width), static_cast<float>(height)));
            
            // 检查是否与已占用区域重叠
            bool overlap = false;
            for (const auto& occupied : _occupiedRegions)
            {
                // 添加padding检查
                Rect paddedOccupied(
                    Point(occupied.getLeft() - padding, occupied.getTop() - padding),
                    Size(occupied.getWidth() + padding * 2, occupied.getHeight() + padding * 2)
                );
                
                if (testRect.intersects(paddedOccupied))
                {
                    overlap = true;
                    break;
                }
            }
            
            if (!overlap)
            {
                outRect = testRect;
                return true;
            }
        }
    }
    
    return false;
}

void GLTextureAtlasPage::markRegionUsed(const Rect& rect)
{
    // 标记网格单元为已使用
    int startGridX = static_cast<int>(rect.getLeft()) / _gridCellSize;
    int startGridY = static_cast<int>(rect.getTop()) / _gridCellSize;
    int endGridX = (static_cast<int>(rect.getRight()) + _gridCellSize - 1) / _gridCellSize;
    int endGridY = (static_cast<int>(rect.getBottom()) + _gridCellSize - 1) / _gridCellSize;
    
    startGridX = std::max(0, startGridX);
    startGridY = std::max(0, startGridY);
    endGridX = std::min(_gridWidth, endGridX);
    endGridY = std::min(_gridHeight, endGridY);
    
    for (int y = startGridY; y < endGridY; ++y)
    {
        for (int x = startGridX; x < endGridX; ++x)
        {
            _usedGrid[y * _gridWidth + x] = true;
        }
    }
}

void GLTextureAtlasPage::copyTextureData(GLTexture* texture, const Rect& destRect)
{
    if (!texture || !_texture)
    {
        return;
    }
    
    // 绑定源纹理为读取目标
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    
    // 使用 glCopyTexSubImage2D 复制纹理数据
    // 首先绑定目标纹理
    glBindTexture(GL_TEXTURE_2D, _textureID);
    
    // 创建临时FBO来读取源纹理
    GLuint tempFBO;
    glGenFramebuffers(1, &tempFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D, texture->getID(), 0);
    
    // 检查FBO完整性
    if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        // 复制像素数据
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
                           static_cast<GLint>(destRect.getLeft()),
                           static_cast<GLint>(destRect.getTop()),
                           0, 0,
                           static_cast<GLsizei>(texture->getWidth()),
                           static_cast<GLsizei>(texture->getHeight()));
    }
    
    // 清理
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &tempFBO);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// GLTextureAtlas 实现

GLTextureAtlas& GLTextureAtlas::getInstance()
{
    static GLTextureAtlas instance;
    return instance;
}

bool GLTextureAtlas::initialize(int pageWidth, int pageHeight, int maxPages)
{
    if (_initialized)
    {
        return true;
    }
    
    _pageWidth = pageWidth;
    _pageHeight = pageHeight;
    _maxPages = maxPages;
    _initialized = true;
    
    E2D_LOG("Texture atlas initialized: page size %dx%d, max pages %d", 
            pageWidth, pageHeight, maxPages);
    return true;
}

void GLTextureAtlas::shutdown()
{
    clear();
    _initialized = false;
}

const AtlasRegion* GLTextureAtlas::addTexture(GLTexture* texture)
{
    if (!_initialized || !texture || !texture->isValid())
    {
        return nullptr;
    }
    
    GLuint texID = texture->getID();
    
    // 检查是否已经在图集中
    auto it = _textureRegions.find(texID);
    if (it != _textureRegions.end())
    {
        return &(it->second);
    }
    
    // 检查纹理是否适合放入图集
    if (!isTextureSuitable(texture))
    {
        return nullptr;
    }
    
    // 尝试将纹理添加到现有页面
    AtlasRegion region;
    for (size_t i = 0; i < _pages.size(); ++i)
    {
        if (_pages[i]->packTexture(texture, region))
        {
            region.pageIndex = static_cast<int>(i);
            _textureRegions[texID] = region;
            return &_textureRegions[texID];
        }
    }
    
    // 现有页面都满了，创建新页面
    if (static_cast<int>(_pages.size()) < _maxPages)
    {
        GLTextureAtlasPage* newPage = createNewPage();
        if (newPage && newPage->packTexture(texture, region))
        {
            region.pageIndex = static_cast<int>(_pages.size()) - 1;
            _textureRegions[texID] = region;
            return &_textureRegions[texID];
        }
    }
    
    E2D_WARNING("Failed to add texture to atlas: texture too large or atlas full");
    return nullptr;
}

bool GLTextureAtlas::removeTexture(GLTexture* texture)
{
    if (!texture)
    {
        return false;
    }
    
    auto it = _textureRegions.find(texture->getID());
    if (it != _textureRegions.end())
    {
        _textureRegions.erase(it);
        return true;
    }
    return false;
}

const AtlasRegion* GLTextureAtlas::getRegion(GLTexture* texture) const
{
    if (!texture)
    {
        return nullptr;
    }
    
    auto it = _textureRegions.find(texture->getID());
    if (it != _textureRegions.end())
    {
        return &(it->second);
    }
    
    return nullptr;
}

bool GLTextureAtlas::containsTexture(GLTexture* texture) const
{
    if (!texture)
    {
        return false;
    }
    
    return _textureRegions.find(texture->getID()) != _textureRegions.end();
}

int GLTextureAtlas::getPageCount() const
{
    return static_cast<int>(_pages.size());
}

GLTextureAtlasPage* GLTextureAtlas::getPage(int index) const
{
    if (index < 0 || index >= static_cast<int>(_pages.size()))
    {
        return nullptr;
    }
    return _pages[index].get();
}

void GLTextureAtlas::clear()
{
    _textureRegions.clear();
    _pages.clear();
}

void GLTextureAtlas::getStats(int& outTotalPages, int& outTotalTextures, float& outAverageUsage) const
{
    outTotalPages = static_cast<int>(_pages.size());
    outTotalTextures = static_cast<int>(_textureRegions.size());
    
    if (_pages.empty())
    {
        outAverageUsage = 0.0f;
        return;
    }
    
    float totalUsage = 0.0f;
    for (const auto& page : _pages)
    {
        totalUsage += page->getUsageRatio();
    }
    outAverageUsage = totalUsage / _pages.size();
}

GLTextureAtlasPage* GLTextureAtlas::createNewPage()
{
    if (static_cast<int>(_pages.size()) >= _maxPages)
    {
        return nullptr;
    }
    
    auto page = std::make_unique<GLTextureAtlasPage>(_pageWidth, _pageHeight);
    if (!page->initialize())
    {
        return nullptr;
    }
    
    _pages.push_back(std::move(page));
    return _pages.back().get();
}

bool GLTextureAtlas::isTextureSuitable(GLTexture* texture) const
{
    if (!texture)
    {
        return false;
    }
    
    // 检查纹理尺寸
    int width = texture->getWidth();
    int height = texture->getHeight();
    
    // 太大或太小的纹理不适合放入图集
    if (width > _maxTextureSize || height > _maxTextureSize)
    {
        return false;
    }
    
    if (width < 1 || height < 1)
    {
        return false;
    }
    
    // 检查纹理是否已经是图集的一部分
    if (containsTexture(texture))
    {
        return false;
    }
    
    return true;
}

} // namespace easy2d

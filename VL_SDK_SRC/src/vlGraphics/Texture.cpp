/**************************************************************************************/
/*                                                                                    */
/*  Visualization Library                                                             */
/*  http://www.visualizationlibrary.com                                               */
/*                                                                                    */
/*  Copyright (c) 2005-2010, Michele Bosi                                             */
/*  All rights reserved.                                                              */
/*                                                                                    */
/*  Redistribution and use in source and binary forms, with or without modification,  */
/*  are permitted provided that the following conditions are met:                     */
/*                                                                                    */
/*  - Redistributions of source code must retain the above copyright notice, this     */
/*  list of conditions and the following disclaimer.                                  */
/*                                                                                    */
/*  - Redistributions in binary form must reproduce the above copyright notice, this  */
/*  list of conditions and the following disclaimer in the documentation and/or       */
/*  other materials provided with the distribution.                                   */
/*                                                                                    */
/*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND   */
/*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE            */
/*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR  */
/*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    */
/*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      */
/*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON    */
/*  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           */
/*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS     */
/*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                      */
/*                                                                                    */
/**************************************************************************************/

#include <vlGraphics/Texture.hpp>
#include <vlCore/checks.hpp>
#include <vlCore/Image.hpp>
#include <vlCore/math3D.hpp>
#include <vlCore/Say.hpp>
#include <vlCore/Log.hpp>

using namespace vl;

namespace
{
  int getDefaultFormat(ETextureFormat internal_format)
  {
    switch(internal_format)
    {
    case TF_DEPTH_STENCIL:
    case TF_DEPTH24_STENCIL8:
    case TF_DEPTH32F_STENCIL8:
    case TF_DEPTH_COMPONENT32F:
    case TF_DEPTH_COMPONENT:
    case TF_DEPTH_COMPONENT16:
    case TF_DEPTH_COMPONENT24:
    case TF_DEPTH_COMPONENT32:
      return GL_DEPTH_COMPONENT;

    case TF_RGBA32UI_EXT:
    case TF_RGB32UI_EXT:
    case TF_ALPHA32UI_EXT:
    case TF_INTENSITY32UI_EXT:
    case TF_LUMINANCE32UI_EXT:
    case TF_LUMINANCE_ALPHA32UI_EXT:

    case TF_RGBA16UI_EXT:
    case TF_RGB16UI_EXT:
    case TF_ALPHA16UI_EXT:
    case TF_INTENSITY16UI_EXT:
    case TF_LUMINANCE16UI_EXT:
    case TF_LUMINANCE_ALPHA16UI_EXT:

    case TF_RGBA8UI_EXT:
    case TF_RGB8UI_EXT:
    case TF_ALPHA8UI_EXT:
    case TF_INTENSITY8UI_EXT:
    case TF_LUMINANCE8UI_EXT:
    case TF_LUMINANCE_ALPHA8UI_EXT:

    case TF_RGBA32I_EXT:
    case TF_RGB32I_EXT:
    case TF_ALPHA32I_EXT:
    case TF_INTENSITY32I_EXT:
    case TF_LUMINANCE32I_EXT:
    case TF_LUMINANCE_ALPHA32I_EXT:

    case TF_RGBA16I_EXT:
    case TF_RGB16I_EXT:
    case TF_ALPHA16I_EXT:
    case TF_INTENSITY16I_EXT:
    case TF_LUMINANCE16I_EXT:
    case TF_LUMINANCE_ALPHA16I_EXT:

    case TF_RGBA8I_EXT:
    case TF_RGB8I_EXT:
    case TF_ALPHA8I_EXT:
    case TF_INTENSITY8I_EXT:
    case TF_LUMINANCE8I_EXT:
    case TF_LUMINANCE_ALPHA8I_EXT:

    case TF_R8I:
    case TF_R8UI:
    case TF_R16I:
    case TF_R16UI:
    case TF_R32I:
    case TF_R32UI:
    case TF_RG8I:
    case TF_RG8UI:
    case TF_RG16I:
    case TF_RG16UI:
    case TF_RG32I:
    case TF_RG32UI:
      return GL_RED_INTEGER;

    default:
      return GL_RED;
    }
  }

  int getDefaultType(ETextureFormat /*internal_format*/)
  {
    return GL_UNSIGNED_BYTE;
  }
}
//-----------------------------------------------------------------------------
// Texture
//-----------------------------------------------------------------------------
void Texture::destroyTexture()
{
  if (mHandle)
    glDeleteTextures(1, &mHandle);
  reset();
  getTexParameter()->mDirty = true;
}
//-----------------------------------------------------------------------------
Texture::~Texture()
{
  destroyTexture();
}
//-----------------------------------------------------------------------------
void Texture::reset()
{
  setDimension(TD_TEXTURE_UNKNOWN);
  setInternalFormat(TF_UNKNOWN);
  setBorder(0);
  setWidth(0);
  setHeight(0);
  setDepth(0);
  mHandle = 0;
  mSetupParams = NULL;
  mBufferObject = NULL;
}
//-----------------------------------------------------------------------------
Texture::Texture(int width, ETextureFormat format, bool border)
{
  VL_CHECK_OGL()
  #ifndef NDEBUG
    mObjectName = className();
  #endif
  reset();
  if (!createTexture(vl::TD_TEXTURE_1D, format, width, 0, 0, border, NULL))
  {
    Log::error("1D texture creation failed!\n");
  }
}
//-----------------------------------------------------------------------------
Texture::Texture(int width, int height, ETextureFormat format, bool border)
{
  VL_CHECK_OGL()
  #ifndef NDEBUG
    mObjectName = className();
  #endif
  reset();
  if (!createTexture(vl::TD_TEXTURE_2D, format, width, height, 0, border, NULL))
  {
    Log::error("2D texture constructor failed!\n");
  }
}
//-----------------------------------------------------------------------------
Texture::Texture(int width, int height, int depth, ETextureFormat format, bool border)
{
  VL_CHECK_OGL()
  #ifndef NDEBUG
    mObjectName = className();
  #endif
  reset();
  if (!createTexture(vl::TD_TEXTURE_3D, format, width, height, depth, border, NULL))
  {
    Log::error("3D texture constructor failed!\n");
  }
}
//-----------------------------------------------------------------------------
Texture::Texture(Image* image, ETextureFormat format, bool mipmaps , bool border)
{
  #ifndef NDEBUG
    mObjectName = className();
  #endif

  reset();

  if (image && image->isValid())
  {
    switch(image->dimension())
    {
    case ID_1D:      prepareTexture1D(image, format, mipmaps, border); break;
    case ID_2D:      prepareTexture2D(image, format, mipmaps, border); break;
    case ID_3D:      prepareTexture3D(image, format, mipmaps, border); break;
    case ID_Cubemap: prepareTextureCubemap(image, format, mipmaps, border); break;
    default:
      break;
    }
    if( !createTexture() )
      Log::error("Texture constructor failed!\n");
  }
  else
    Log::bug("Texture constructor called with an invalid Image!\n");
}
//-----------------------------------------------------------------------------
Texture::Texture(const String& image_path, ETextureFormat format, bool mipmaps , bool border)
{
  #ifndef NDEBUG
    mObjectName = className();
  #endif

  reset();

  ref<Image> image = vl::loadImage(image_path);

  if (image && image->isValid())
  {
    switch(image->dimension())
    {
    case ID_1D:      prepareTexture1D(image.get(), format, mipmaps, border); break;
    case ID_2D:      prepareTexture2D(image.get(), format, mipmaps, border); break;
    case ID_3D:      prepareTexture3D(image.get(), format, mipmaps, border); break;
    case ID_Cubemap: prepareTextureCubemap(image.get(), format, mipmaps, border); break;
    default:
      break;
    }
    if( !createTexture() )
      Log::error("Texture constructor failed!\n");
  }
  else
    Log::bug("Texture constructor called with an invalid Image!\n");
}
//-----------------------------------------------------------------------------
Texture::Texture()
{
  #ifndef NDEBUG
    mObjectName = className();
  #endif
  reset();
}
//-----------------------------------------------------------------------------
bool Texture::isValid() const
{
  bool a = mWidth != 0 && mHeight == 0 && mDepth == 0;
  bool b = mWidth != 0 && mHeight != 0 && mDepth == 0;
  bool c = mWidth != 0 && mHeight != 0 && mDepth != 0;
  return handle() != 0 && (a|b|c);
}
//-----------------------------------------------------------------------------
bool Texture::supports(ETextureDimension tex_dimension, ETextureFormat tex_format, int mip_level, EImageDimension img_dimension, int w, int h, int d, bool border, bool verbose)
{
  VL_CHECK_OGL();

  // clear errors

  glGetError();

  // texture buffer

  if ( tex_dimension == TD_TEXTURE_BUFFER )
  {
    if (!(GLEW_ARB_texture_buffer_object||GLEW_EXT_texture_buffer_object||GLEW_VERSION_3_1||GLEW_VERSION_4_0))
    {
      if (verbose) Log::error("Texture::supports(): texture buffer not supported by the current hardware.\n");
      return false;
    }

    if (border)
    {
      if (verbose) Log::error("Texture::supports(): a texture buffer cannot have borders.\n");
      return false;
    }

    // these should be zero
    VL_CHECK( !(w||h||d) );
  }

  // cubemaps

  if ( tex_dimension == TD_TEXTURE_CUBE_MAP )
  {
    if (!(GLEW_ARB_texture_cube_map||GLEW_VERSION_1_3||GLEW_VERSION_3_0))
    {
      if (verbose) Log::error("Texture::supports(): texture cubemap not supported by the current hardware.\n");
      return false;
    }

    if ( w != h )
    {
      if (verbose) Log::error("Texture::supports(): cubemaps must have square faces.\n");
      return false;
    }
  }

  // texture arrays

  if ( tex_dimension == TD_TEXTURE_1D_ARRAY || tex_dimension == TD_TEXTURE_2D_ARRAY )
  {
    if (border)
    {
      if (verbose) Log::error("Texture::supports(): you cannot create a texture array with borders.\n");
      return false;
    }

    if(!(GLEW_EXT_texture_array||GLEW_VERSION_3_0))
    {
      if (verbose) Log::error("Texture::supports(): texture array not supported by the current hardware.\n");
      return false;
    }

    if ( img_dimension )
    {
      if ( (img_dimension != ID_2D && tex_dimension == TD_TEXTURE_1D_ARRAY) || ( img_dimension != ID_3D && tex_dimension == TD_TEXTURE_2D_ARRAY ) )
      {
        if (verbose) Log::error("Texture::supports(): the image dimensions are not suitable to create a texture array."
          "To create a 1D texture array you need a 2D image and to create a 2D texture array you need a 3D image.\n");
        return false;
      }
    }
  }

  // texture rectangle

  if (tex_dimension == TD_TEXTURE_RECTANGLE)
  {
    if (!(GLEW_ARB_texture_rectangle||GLEW_EXT_texture_rectangle||GLEW_NV_texture_rectangle||GLEW_VERSION_3_1))
    {
      if (verbose) Log::error("Texture::supports(): texture rectangle not supported by the current hardware.\n");
      return false;
    }

    if ( mip_level != 0 )
    {
      if (verbose) Log::error("Texture::supports(): TD_TEXTURE_RECTANGLE textures do not support mipmapping level other than zero.\n");
      return false;
    }

    if (border)
    {
      if (verbose) Log::error("Texture::supports(): TD_TEXTURE_RECTANGLE textures do not allow textures borders\n");
      return false;
    }
  }

  int width = 0;

  if (tex_dimension == TD_TEXTURE_BUFFER)
  {
    width = 1; // pass the test
  }
  else
  if (tex_dimension == TD_TEXTURE_CUBE_MAP)
  {
    glTexImage2D(GL_PROXY_TEXTURE_CUBE_MAP, mip_level, tex_format, w + (border?2:0), h + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_CUBE_MAP, mip_level, GL_TEXTURE_WIDTH, &width);
  }
  else
  if (tex_dimension == TD_TEXTURE_2D_ARRAY)
  {
    glTexImage3D(GL_PROXY_TEXTURE_2D_ARRAY, mip_level, tex_format, w + (border?2:0), h + (border?2:0), d + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D_ARRAY, mip_level, GL_TEXTURE_WIDTH, &width);
  }
  else
  if (tex_dimension == TD_TEXTURE_3D)
  {
    glTexImage3D(GL_PROXY_TEXTURE_3D, mip_level, tex_format, w + (border?2:0), h + (border?2:0), d + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_3D, mip_level, GL_TEXTURE_WIDTH, &width);
  }
  else
  if (tex_dimension == TD_TEXTURE_RECTANGLE)
  {
    glTexImage2D(GL_PROXY_TEXTURE_RECTANGLE, mip_level, tex_format, w, h, 0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_RECTANGLE, mip_level, GL_TEXTURE_WIDTH, &width);
  }
  else
  if (tex_dimension == TD_TEXTURE_1D_ARRAY)
  {
    glTexImage2D(GL_PROXY_TEXTURE_1D_ARRAY, mip_level, tex_format, w, h, 0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_1D_ARRAY, mip_level, GL_TEXTURE_WIDTH, &width);
  }
  else
  if (tex_dimension == TD_TEXTURE_2D)
  {
    glTexImage2D(GL_PROXY_TEXTURE_2D, mip_level, tex_format, w + (border?2:0), h + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, mip_level, GL_TEXTURE_WIDTH, &width);
  }
  else
  if (tex_dimension == TD_TEXTURE_1D)
  {
    glTexImage1D(GL_PROXY_TEXTURE_1D, mip_level, tex_format, w + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
    glGetTexLevelParameteriv(GL_PROXY_TEXTURE_1D, mip_level, GL_TEXTURE_WIDTH, &width);
  }

  GLenum err = glGetError();
  return err == 0 && width != 0;
}
//-----------------------------------------------------------------------------
bool Texture::createTexture(ETextureDimension tex_dimension, ETextureFormat tex_format, int w, int h, int d, bool border, GLBufferObject* buffer_object)
{
  VL_CHECK_OGL()

  if ( tex_dimension == TD_TEXTURE_BUFFER )
  {
    if( !buffer_object || !buffer_object->handle() || !glIsBuffer(buffer_object->handle()) )
    {
      Log::bug( "Texture::createTexture() requires a non NULL valid buffer object in order to create a texture buffer!\n" );
      VL_CHECK(buffer_object);
      VL_CHECK(buffer_object->handle());
      VL_CHECK(glIsBuffer(buffer_object->handle()));
      return false;
    }
    else
    {
      // the buffer object must not be empty!
      GLint buffer_size = 0;
      glBindBuffer(GL_TEXTURE_BUFFER, buffer_object->handle());
      glGetBufferParameteriv(GL_TEXTURE_BUFFER, GL_BUFFER_SIZE, &buffer_size);
      glBindBuffer(GL_TEXTURE_BUFFER, 0);
      if ( buffer_size == 0 )
      {
        Log::bug("Texture::createTexture(): cannot create a texture buffer with an empty buffer object!\n"); VL_TRAP();
        return false;
      }
    }
  }

  if (mHandle)
  {
    Log::bug("Texture::createTexture(): a texture can be created only once!\n");
    return false;
  }
  else
  {
    if ( !supports(tex_dimension , tex_format, 0, ID_None, w, h, d, border, true) )
    {
      VL_CHECK_OGL()
      Log::bug("Texture::createTexture(): the format/size combination requested is not supported!\n"); VL_TRAP();
      return false;
    }

    reset();

    glGenTextures( 1, &mHandle ); VL_CHECK_OGL();

    if (!mHandle)
    {
      Log::bug("Texture::createTexture(): texture creation failed!\n");
      VL_TRAP();
      return false;
    }

    setDimension(tex_dimension);
    setInternalFormat(tex_format);
    setWidth(w);
    setHeight(h);
    setDepth(d);
    setBorder(border);
    mBufferObject = buffer_object; // the user cannot change this

    glBindTexture(tex_dimension, mHandle); VL_CHECK_OGL();

    if (tex_dimension == TD_TEXTURE_BUFFER)
    {
      VL_CHECK(buffer_object)
      VL_CHECK(buffer_object->handle())
      glTexBuffer(GL_TEXTURE_BUFFER, tex_format, buffer_object->handle());
      unsigned int glerr = glGetError();
      if (glerr != GL_NO_ERROR)
      {
        String msg( (const char*)gluErrorString(glerr) );
        Log::bug( "Texture::createTexture(): glTexBuffer() failed with error: '" + msg + "'.\n" );
        Log::error("Probably you supplied a non supported texture format! Review the glTexBuffer() man page for a complete list of supported texture formats.\n");
        VL_TRAP();
      }
    }
    else
    if (tex_dimension == TD_TEXTURE_CUBE_MAP)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, tex_format, w + (border?2:0), h + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, tex_format, w + (border?2:0), h + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, tex_format, w + (border?2:0), h + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, tex_format, w + (border?2:0), h + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, tex_format, w + (border?2:0), h + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, tex_format, w + (border?2:0), h + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      VL_CHECK_OGL();
    }
    else
    if (tex_dimension == TD_TEXTURE_2D_ARRAY)
    {
      glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, tex_format, w + (border?2:0), h + (border?2:0), d + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      VL_CHECK_OGL();
    }
    else
    if (tex_dimension == TD_TEXTURE_3D)
    {
      glTexImage3D(GL_TEXTURE_3D, 0, tex_format, w + (border?2:0), h + (border?2:0), d + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      VL_CHECK_OGL();
    }
    else
    if (tex_dimension == TD_TEXTURE_RECTANGLE)
    {
      glTexImage2D(GL_TEXTURE_RECTANGLE, 0, tex_format, w, h, 0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      VL_CHECK_OGL();
    }
    else
    if (tex_dimension == TD_TEXTURE_1D_ARRAY)
    {
      glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, tex_format, w, h, 0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      VL_CHECK_OGL();
    }
    else
    if (tex_dimension == TD_TEXTURE_2D)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, tex_format, w + (border?2:0), h + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      VL_CHECK_OGL();
    }
    else
    if (tex_dimension == TD_TEXTURE_1D)
    {
      glTexImage1D(GL_TEXTURE_1D, 0, tex_format, w + (border?2:0), border?1:0, getDefaultFormat(tex_format), getDefaultType(tex_format), NULL);
      VL_CHECK_OGL();
    }

    glBindTexture(tex_dimension, 0); VL_CHECK_OGL();
    return true;
  }
}
//-----------------------------------------------------------------------------
bool Texture::setMipLevel(int mip_level, Image* img, bool gen_mipmaps)
{
  VL_CHECK_OGL()

  if (!mHandle)
  {
    Log::error("Texture::setMipLevel(): texture hasn't been created yet, please call createTexture() first!\n");
    VL_TRAP();
    return false;
  }

  if ( !supports(dimension(), internalFormat(), mip_level, img->dimension(), img->width(), img->height(), img->depth(), border(), true) )
  {
    VL_CHECK_OGL()
    Log::error("Texture::setMipLevel(): the format/size combination requested is not supported.\n");
    return false;
  }

  glPixelStorei( GL_UNPACK_ALIGNMENT, img->byteAlignment() ); VL_CHECK_OGL()

  glBindTexture( dimension(), mHandle ); VL_CHECK_OGL()

  int w = width()  + (border()?2:0);
  int h = height() + (border()?2:0);
  int d = depth()  + (border()?2:0);
  int is_compressed = (int)img->format() == (int)internalFormat() && isCompressedFormat( internalFormat() );

  bool use_glu = false;
  GLint generate_mipmap_orig = GL_FALSE;
  if ( gen_mipmaps )
  {
    if( GLEW_SGIS_generate_mipmap||GLEW_VERSION_1_4||GLEW_VERSION_3_0 )
    {
      glGetTexParameteriv( dimension(), GL_GENERATE_MIPMAP, &generate_mipmap_orig ); VL_CHECK_OGL()
      glTexParameteri(dimension(), GL_GENERATE_MIPMAP, GL_TRUE); VL_CHECK_OGL()
    }
    else
    {
      if (mip_level > 0) // because GLU regenerates the whole mip-mapping chain
        Log::error("Texture::setMipLevel(): automatic mipmaps generation for levels below 0 requires OpenGL 1.4 minimum.\n");
      else
        use_glu = true;

      #define VL_IS_POW_2(x) ((x != 0) && ((x & (x - 1)) == 0))
      if ( !VL_IS_POW_2(w) || !VL_IS_POW_2(h) )
        Log::warning("Texture::setMipLevel(): the image will be rescaled to the nearest upper power of 2.\n");
    }
  }

  if ( use_glu && is_compressed )
  {
    Log::error("Texture::setMipLevel(): could not generate compressed mipmaps, OpenGL 1.4 required.\n");
    use_glu = false;
  }

  if ( use_glu && dimension() == TD_TEXTURE_3D )
  {
    Log::error("Texture::setMipLevel(): could not generate 3D mipmaps, OpenGL 1.4 required.\n");
    use_glu = false;
  }

  if (dimension() == TD_TEXTURE_CUBE_MAP)
  {
    if (is_compressed)
    {
      VL_CHECK( img->requiredMemory() % 6 == 0 );
      int bytes = img->requiredMemory() / 6;
      glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, mip_level, internalFormat(), w, h, border()?1:0, bytes, img->pixelsXP());
      glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, mip_level, internalFormat(), w, h, border()?1:0, bytes, img->pixelsXN());
      glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, mip_level, internalFormat(), w, h, border()?1:0, bytes, img->pixelsYP());
      glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, mip_level, internalFormat(), w, h, border()?1:0, bytes, img->pixelsYN());
      glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, mip_level, internalFormat(), w, h, border()?1:0, bytes, img->pixelsZP());
      glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, mip_level, internalFormat(), w, h, border()?1:0, bytes, img->pixelsZN());
      VL_CHECK_OGL()
    }
    else
    {
      if (use_glu)
      {
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, internalFormat(), w, h, img->format(), img->type(), img->pixelsXP());
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, internalFormat(), w, h, img->format(), img->type(), img->pixelsXN());
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, internalFormat(), w, h, img->format(), img->type(), img->pixelsYP());
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, internalFormat(), w, h, img->format(), img->type(), img->pixelsYN());
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, internalFormat(), w, h, img->format(), img->type(), img->pixelsZP());
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, internalFormat(), w, h, img->format(), img->type(), img->pixelsZN());
        VL_CHECK_OGL()
      }
      else
      {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, mip_level, internalFormat(), w, h, border()?1:0, img->format(), img->type(), img->pixelsXP());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, mip_level, internalFormat(), w, h, border()?1:0, img->format(), img->type(), img->pixelsXN());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, mip_level, internalFormat(), w, h, border()?1:0, img->format(), img->type(), img->pixelsYP());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, mip_level, internalFormat(), w, h, border()?1:0, img->format(), img->type(), img->pixelsYN());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, mip_level, internalFormat(), w, h, border()?1:0, img->format(), img->type(), img->pixelsZP());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, mip_level, internalFormat(), w, h, border()?1:0, img->format(), img->type(), img->pixelsZN());
        VL_CHECK_OGL()
      }
    }
  }
  else
  if (dimension() == TD_TEXTURE_2D_ARRAY)
  {
    if (is_compressed)
    {
      glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, mip_level, internalFormat(), w, h, d, border()?1:0, img->requiredMemory(), img->pixels());
      VL_CHECK_OGL()
    }
    else
    {
      glTexImage3D(GL_TEXTURE_2D_ARRAY, mip_level, internalFormat(), w, h, d, border()?1:0, img->format(), img->type(), img->pixels());
      VL_CHECK_OGL()
    }
  }
  else
  if (dimension() == TD_TEXTURE_3D)
  {
    if (is_compressed)
    {
      glCompressedTexImage3D(GL_TEXTURE_3D, mip_level, internalFormat(), w, h, d, border()?1:0, img->requiredMemory(), img->pixels());
      VL_CHECK_OGL()
    }
    else
    {
      glTexImage3D(GL_TEXTURE_3D, mip_level, internalFormat(), w, h, d, border()?1:0, img->format(), img->type(), img->pixels());
      VL_CHECK_OGL()
    }
  }
  else
  if (dimension() == TD_TEXTURE_RECTANGLE)
  {
    if (is_compressed)
    {
      glCompressedTexImage2D(GL_TEXTURE_RECTANGLE, mip_level, internalFormat(), width(), height(), 0, img->requiredMemory(), img->pixels());
      VL_CHECK_OGL()
    }
    else
    {
      glTexImage2D(GL_TEXTURE_RECTANGLE, mip_level, internalFormat(), width(), height(), 0, img->format(), img->type(), img->pixels());
      VL_CHECK_OGL()
    }
  }
  else
  if (dimension() == TD_TEXTURE_1D_ARRAY)
  {
    if (is_compressed)
    {
      glCompressedTexImage2D(GL_TEXTURE_1D_ARRAY, mip_level, internalFormat(), width(), height(), 0, img->requiredMemory(), img->pixels());
      VL_CHECK_OGL()
    }
    else
    {
      glTexImage2D(GL_TEXTURE_1D_ARRAY, mip_level, internalFormat(), width(), height(), 0, img->format(), img->type(), img->pixels());
      VL_CHECK_OGL()
    }
  }
  else
  if (dimension() == TD_TEXTURE_2D)
  {
    if (is_compressed)
    {
      glCompressedTexImage2D(GL_TEXTURE_2D, mip_level, internalFormat(), w, h, border()?1:0, img->requiredMemory(), img->pixels());
      VL_CHECK_OGL()
    }
    else
    {
      if (use_glu)
      {
        gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat(), w, h, img->format(), img->type(), img->pixels());
        VL_CHECK_OGL()
      }
      else
      {
        glTexImage2D(GL_TEXTURE_2D, mip_level, internalFormat(), w, h, border()?1:0, img->format(), img->type(), img->pixels());
        VL_CHECK_OGL()
      }
    }
  }
  else
  if (dimension() == TD_TEXTURE_1D)
  {
    if (is_compressed)
    {
      glCompressedTexImage1D(GL_TEXTURE_1D, mip_level, internalFormat(), w, border()?1:0, img->requiredMemory(), img->pixels());
      VL_CHECK_OGL()
    }
    else
    {
      if (use_glu)
      {
        gluBuild1DMipmaps(GL_TEXTURE_1D, internalFormat(), w, img->format(), img->type(), img->pixels());
        VL_CHECK_OGL()
      }
      else
      {
        glTexImage1D(GL_TEXTURE_1D, mip_level, internalFormat(), w, border()?1:0, img->format(), img->type(), img->pixels());
        VL_CHECK_OGL()
      }
    }
  }

  if ( gen_mipmaps && (GLEW_SGIS_generate_mipmap||GLEW_VERSION_1_4||GLEW_VERSION_3_0) )
  {
    glTexParameteri(dimension(), GL_GENERATE_MIPMAP, generate_mipmap_orig);
    VL_CHECK_OGL()
  }

  glBindTexture( dimension(), 0 ); VL_CHECK_OGL()

  glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

  return true;
}
//-----------------------------------------------------------------------------
bool Texture::createTexture()
{
  VL_CHECK_OGL()

  if (!setupParams())
    return false;

  class InOutCondition
  {
    Texture* mTex;
  public:
    InOutCondition(Texture* tex): mTex(tex) {}
    ~InOutCondition() 
    {
      // make sure no errors were generated
      VL_CHECK_OGL()

      // uninstall setup parameters
      mTex->mSetupParams = NULL;
    }
  };

  InOutCondition in_out_condition(this);

  ETextureFormat tex_format = setupParams()->format();
  ETextureDimension tex_dimension = setupParams()->dimension();
  bool gen_mipmaps = setupParams()->genMipmaps();
  bool border = setupParams()->border();
  ref<Image> img = setupParams()->image();
  if ( !img && !setupParams()->imagePath().empty() )
  {
    img = loadImage( setupParams()->imagePath() );
    if (!img)
    {
      vl::Log::error( Say("Texture::createTexture(): could not load image file '%s'\n") << setupParams()->imagePath() );
      return false;
    }
  }

  int w = setupParams()->width();
  int h = setupParams()->height();
  int d = setupParams()->depth();
  if (img)
  {
    setObjectName( img->objectName() );
    w = img->width();
    h = img->height();
    d = img->depth();
  }
  //w = w > 0 ? w : 1;
  //h = h > 0 ? h : 1;
  //d = d > 0 ? d : 1;

  if ( !createTexture(tex_dimension, tex_format, w, h, d, border, setupParams()->bufferObject()) )
    return false;

  VL_CHECK_OGL()

  if (img)
  {
    // compile mipmapping levels
    std::vector<vl::Image*> mipmaps;
    mipmaps.push_back(img.get());
    for(int i=0; i<(int)img->mipmaps().size(); ++i)
      mipmaps.push_back( img->mipmaps()[i].get() );

    bool ok = false;

    if (!gen_mipmaps) // no mipmaps
      ok = setMipLevel(0, img.get(), false);
    else 
    {
      if (mipmaps.size() > 1) // explicit mipmaps
      {
        for(int i=0; i<(int)mipmaps.size(); ++i)
          ok = setMipLevel(i, mipmaps[i], false);
      }
      else // automatic mipmaps generation
      if (mipmaps.size() == 1)
      {
        ok = setMipLevel(0, img.get(), true);
      }
    }
    return ok;
  }
  else
    return true;
}
//-----------------------------------------------------------------------------
bool Texture::isCompressedFormat(int format)
{
  int comp[] =
  {
    TF_COMPRESSED_ALPHA,
    TF_COMPRESSED_INTENSITY,
    TF_COMPRESSED_LUMINANCE,
    TF_COMPRESSED_LUMINANCE_ALPHA,
    TF_COMPRESSED_RGB,
    TF_COMPRESSED_RGBA,

    // 3DFX_texture_compression_FXT1
    TF_COMPRESSED_RGB_FXT1_3DFX,
    TF_COMPRESSED_RGBA_FXT1_3DFX,

    // EXT_texture_compression_s3tc
    TF_COMPRESSED_RGB_S3TC_DXT1_EXT,
    TF_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    TF_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    TF_COMPRESSED_RGBA_S3TC_DXT5_EXT,

    // GL_EXT_texture_compression_latc
    TF_COMPRESSED_LUMINANCE_LATC1_EXT,
    TF_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT,
    TF_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,
    TF_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT,

    // EXT_texture_compression_rgtc
    TF_COMPRESSED_RED_RGTC1_EXT,
    TF_COMPRESSED_SIGNED_RED_RGTC1_EXT,
    TF_COMPRESSED_RED_GREEN_RGTC2_EXT,
    TF_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT,

    0
  };

  for(int i=0; comp[i]; ++i)
  {
    if(comp[i] == format)
      return true;
  }

  return false;
}
//-----------------------------------------------------------------------------

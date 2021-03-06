/* Framework for Live Image Transformation (FLITr)
 * Copyright (c) 2010 CSIR
 *
 * This file is part of FLITr.
 *
 * FLITr is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * FLITr is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FLITr. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <flitr/modules/flitr_image_processors/cnvrt_to_8bit/fip_cnvrt_to_rgb_8.h>


using namespace flitr;
using std::shared_ptr;

FIPConvertToRGB8::FIPConvertToRGB8(ImageProducer& upStreamProducer, uint32_t images_per_slot,
                                   float scale_factor,
                                   uint32_t buffer_size) :
ImageProcessor(upStreamProducer, images_per_slot, buffer_size),
scaleFactor_(scale_factor),
Title_("Convert To RGB8")
{
    
    //Setup image format being produced to downstream.
    for (uint32_t i=0; i<images_per_slot; i++)
    {
        ImageFormat downStreamFormat(upStreamProducer.getFormat().getWidth(), upStreamProducer.getFormat().getHeight(),
                                     ImageFormat::FLITR_PIX_FMT_RGB_8);
        
        ImageFormat_.push_back(downStreamFormat);
    }
    
}

FIPConvertToRGB8::~FIPConvertToRGB8()
{
}

bool FIPConvertToRGB8::init()
{
    bool rValue=ImageProcessor::init();
    //Note: SharedImageBuffer of downstream producer is initialised with storage in ImageProcessor::init.
    
    return rValue;
}

bool FIPConvertToRGB8::trigger()
{
    if ((getNumReadSlotsAvailable())&&(getNumWriteSlotsAvailable()))
    {//There are images to consume and the downstream producer has space to produce.
        std::vector<Image**> imvRead=reserveReadSlot();
        std::vector<Image**> imvWrite=reserveWriteSlot();
        
        //Start stats measurement event.
        ProcessorStats_->tick();
        
        for (size_t imgNum=0; imgNum<ImagesPerSlot_; ++imgNum)
        {
            Image const * const imRead = *(imvRead[imgNum]);
            Image * const imWrite = *(imvWrite[imgNum]);
            
            uint8_t * const dataWrite=imWrite->data();
            
            const ImageFormat imFormatUS=getUpstreamFormat(imgNum);
            
            const size_t width=imFormatUS.getWidth();
            const size_t height=imFormatUS.getHeight();
            
            if (imFormatUS.getPixelFormat()==ImageFormat::FLITR_PIX_FMT_RGB_F32)
            {
                float const * const dataRead=(float *)imRead->data();
                
                for (size_t y=0; y<height; ++y)
                {
                    size_t offset=(y * width) * 3;
                    
                    for (size_t x=0; x<width; ++x)
                    {
                        const float R=dataRead[offset + 0]*(256.0f*scaleFactor_);
                        const float G=dataRead[offset + 1]*(256.0f*scaleFactor_);
                        const float B=dataRead[offset + 2]*(256.0f*scaleFactor_);
                        
                        dataWrite[offset + 0]=(R>=255.0f)?((uint8_t)255):((R<=0.0f)?((uint8_t)0):(R+0.5f));
                        dataWrite[offset + 1]=(G>=255.0f)?((uint8_t)255):((G<=0.0f)?((uint8_t)0):(G+0.5f));
                        dataWrite[offset + 2]=(B>=255.0f)?((uint8_t)255):((B<=0.0f)?((uint8_t)0):(B+0.5f));
                        
                        offset+=3;
                    }
                }
            } else
            if (imFormatUS.getPixelFormat()==ImageFormat::FLITR_PIX_FMT_Y_F32)
            {
                float const * const dataRead=(float *)imRead->data();
                
                for (size_t y=0; y<height; ++y)
                {
                    const size_t readOffset=(y * width);
                    size_t writeOffset=readOffset * 3;
                    
                    for (size_t x=0; x<width; ++x)
                    {
                        const float I=dataRead[readOffset+x]*(256.0f*scaleFactor_);
                        const float clampedI=(I>=255.0f)?((uint8_t)255):((I<=0.0f)?((uint8_t)0):(I+0.5f));
                        
                        dataWrite[writeOffset + 0]=clampedI;
                        dataWrite[writeOffset + 1]=clampedI;
                        dataWrite[writeOffset + 2]=clampedI;
                        
                        writeOffset+=3;
                    }
                }
            } else
            if (imFormatUS.getPixelFormat()==ImageFormat::FLITR_PIX_FMT_Y_8)
            {
                uint8_t const * const dataRead=imRead->data();
                
                for (size_t y=0; y<height; ++y)
                {
                    const size_t readOffset=(y * width);
                    size_t writeOffset=readOffset * 3;
                    
                    for (size_t x=0; x<width; ++x)
                    {
                        const float I=dataRead[readOffset+x];
                        const float clampedI=(I>=255)?((uint8_t)255):((I<=0)?((uint8_t)0):(I+1));
                        
                        dataWrite[writeOffset + 0]=clampedI;
                        dataWrite[writeOffset + 1]=clampedI;
                        dataWrite[writeOffset + 2]=clampedI;
                        
                        writeOffset+=3;
                    }
                }
			} else
            if (imFormatUS.getPixelFormat()==ImageFormat::FLITR_PIX_FMT_Y_16)
            {
                uint16_t const * const dataRead=(uint16_t *)imRead->data();
                
                for (size_t y=0; y<height; ++y)
                {
                    const size_t readOffset=(y * width);
                    size_t writeOffset=readOffset * 3;
                    
                    for (size_t x=0; x<width; ++x)
                    {
                        const float I=dataRead[readOffset+x];
                        const float clampedI=(I>=255)?((uint8_t)255):((I<=0)?((uint8_t)0):(I+1));
                        
                        dataWrite[writeOffset + 0]=clampedI;
                        dataWrite[writeOffset + 1]=clampedI;
                        dataWrite[writeOffset + 2]=clampedI;
                        
                        writeOffset+=3;
                    }
                }
			}
        }
        
        //Stop stats measurement event.
        ProcessorStats_->tock();
        
        releaseWriteSlot();
        releaseReadSlot();
        
        return true;
    }
    
    return false;
}


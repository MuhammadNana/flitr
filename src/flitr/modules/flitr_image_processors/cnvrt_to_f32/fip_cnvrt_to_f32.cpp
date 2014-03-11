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

#include <flitr/modules/flitr_image_processors/cnvrt_to_f32/fip_cnvrt_to_f32.h>

using namespace flitr;
using std::tr1::shared_ptr;

FIPConvertToF32::FIPConvertToF32(ImageProducer& upStreamProducer, uint32_t images_per_slot,
                                 uint32_t buffer_size) :
ImageProcessor(upStreamProducer, images_per_slot, buffer_size)
{
    
    //Setup image format being produced to downstream.
    for (uint32_t i=0; i<images_per_slot; i++) {
        //ImageFormat(uint32_t w=0, uint32_t h=0, PixelFormat pix_fmt=FLITR_PIX_FMT_Y_8, bool flipV = false, bool flipH = false):
        ImageFormat downStreamFormat(upStreamProducer.getFormat().getWidth(), upStreamProducer.getFormat().getHeight(),
                                     ImageFormat::FLITR_PIX_FMT_Y_F32);
        
        ImageFormat_.push_back(downStreamFormat);
    }
    
}

FIPConvertToF32::~FIPConvertToF32()
{
}

bool FIPConvertToF32::init()
{
    bool rValue=ImageProcessor::init();
    //Note: SharedImageBuffer of downstream producer is initialised with storage in ImageProcessor::init.
    
    return rValue;
}

bool FIPConvertToF32::trigger()
{
    if ((getNumReadSlotsAvailable())&&(getNumWriteSlotsAvailable()))
    {//There are images to consume and the downstream producer has space to produce.
        std::vector<Image**> imvRead=reserveReadSlot();
        std::vector<Image**> imvWrite=reserveWriteSlot();
        
        //Start stats measurement event.
        ProcessorStats_->tick();
        
        for (size_t imgNum=0; imgNum<ImagesPerSlot_; imgNum++)
        {
            Image const * const imRead = *(imvRead[imgNum]);
            Image * const imWrite = *(imvWrite[imgNum]);
            
            uint8_t const * const dataRead=imRead->data();
            
            float * const dataWrite=(float *)imWrite->data();
            
            const ImageFormat imFormat=getDownstreamFormat(imgNum);
            
            const size_t width=imFormat.getWidth();
            const size_t height=imFormat.getHeight();
            const size_t componentsPerPixel=imFormat.getComponentsPerPixel();
            
            const size_t componentsPerLine=componentsPerPixel * width;
            
            
            int y=0;
            {
                {
                    for (y=0; y<(int)height; y++)
                    {
                        const size_t lineOffset=y * componentsPerLine;
                        
                        for (size_t compNum=0; compNum<componentsPerLine; compNum++)
                        {
                            dataWrite[lineOffset + compNum]=dataRead[lineOffset + compNum] * 0.00390625f; // /256.0
                        }
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


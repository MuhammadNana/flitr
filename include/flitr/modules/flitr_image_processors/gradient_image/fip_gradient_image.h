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

#ifndef FIP_GRADIENT_IMAGE_H
#define FIP_GRADIENT_IMAGE_H 1

#include <flitr/image_processor.h>

namespace flitr {
    
    /*! Calculates the image gradient in the x direction using the Shcarr rotationally symmetric operator. */
    class FLITR_EXPORT FIPGradientXImage : public ImageProcessor
    {
    public:
        
        /*! Constructor given the upstream producer.
         *@param producer The upstream image producer.
         *@param images_per_slot The number of images per image slot from the upstream producer.
         *@param buffer_size The size of the shared image buffer of the downstream producer.*/
        FIPGradientXImage(ImageProducer& upStreamProducer, uint32_t images_per_slot,
                          uint32_t buffer_size=FLITR_DEFAULT_SHARED_BUFFER_NUM_SLOTS);
        
        /*! Virtual destructor */
        virtual ~FIPGradientXImage();
        
        /*! Method to initialise the object.
         *@return Boolean result flag. True indicates successful initialisation.*/
        virtual bool init();
        
        /*!Synchronous trigger method. Called automatically by the trigger thread if started.
         *@sa ImageProcessor::startTriggerThread*/
        virtual bool trigger();
        
    private:
    };
    
    /*! Calculates the image gradient in the y direction using the Shcarr rotationally symmetric operator. */
    class FLITR_EXPORT FIPGradientYImage : public ImageProcessor
    {
    public:
        
        /*! Constructor given the upstream producer.
         *@param producer The upstream image producer.
         *@param images_per_slot The number of images per image slot from the upstream producer.
         *@param buffer_size The size of the shared image buffer of the downstream producer.*/
        FIPGradientYImage(ImageProducer& upStreamProducer, uint32_t images_per_slot,
                          uint32_t buffer_size=FLITR_DEFAULT_SHARED_BUFFER_NUM_SLOTS);
        
        /*! Virtual destructor */
        virtual ~FIPGradientYImage();
        
        /*! Method to initialise the object.
         *@return Boolean result flag. True indicates successful initialisation.*/
        virtual bool init();
        
        /*!Synchronous trigger method. Called automatically by the trigger thread if started.
         *@sa ImageProcessor::startTriggerThread*/
        virtual bool trigger();
        
    private:
    };
    
}

#endif //FIP_GRADIENT_IMAGE_H

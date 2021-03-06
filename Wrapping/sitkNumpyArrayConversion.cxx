/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/
#include <string.h>

#include "sitkImage.h"
#include "sitkConditional.h"
#include "sitkExceptionObject.h"

namespace sitk = itk::simple;

// Python is written in C
#ifdef __cplusplus
extern "C"
{
#endif

/** An internal function that extracts the data an information to be used by the
 * interpretted GetArrayFromImage to create the numpy array.  It avoids needing
 * numpy available at compile time.  This is not intended to be called directly.
 *
 * This return a tuple of a Python bytearray with the raw data, a Python tuple
 * with the shape, and an integer indicating the dtype. */
static PyObject *
sitk_GetArrayFromImage( PyObject *SWIGUNUSEDPARM(self), PyObject *args )
{
  // Holds the return tuple
  PyObject * resultTuple = NULL;
  // Holds the bulk data
  PyObject * byteArray = NULL;
  // Numpy array shape
  PyObject * shape = NULL;
  // An integer used by the interpreted caller to identify the pixel type
  // (dtype)
  PyObject * pixelID = NULL;

  const void * sitkBufferPtr;
  Py_ssize_t len;
  typedef std::vector< unsigned int > SizeType;
  SizeType size;
  size_t pixelSize;
  sitk::PixelIDValueType pixelIDValue;

  unsigned int dimension;

  /* Cast over to a sitk Image. */
  PyObject * pyImage;
  void * voidImage;
  const sitk::Image * sitkImage;
  int res = 0;
  if( !PyArg_ParseTuple( args, "O", &pyImage ) )
    {
    SWIG_fail; // SWIG_fail is a macro that says goto: fail (return NULL)
    }
  res = SWIG_ConvertPtr( pyImage, &voidImage, SWIGTYPE_p_itk__simple__Image, 0 );
  if( !SWIG_IsOK( res ) )
    {
    SWIG_exception_fail(SWIG_ArgError(res), "in method 'GetArrayFromImage', argument needs to be of type 'sitk::Image *'");
    }
  sitkImage = reinterpret_cast< sitk::Image * >( voidImage );

  pixelIDValue = sitkImage->GetPixelIDValue();
  pixelSize = 1;
  switch( pixelIDValue )
    {
  case sitk::sitkUnknown:
    PyErr_SetString( PyExc_RuntimeError, "Unknown pixel type." );
    SWIG_fail;
    break;
  case sitk::ConditionalValue< sitk::sitkUInt8 != sitk::sitkUnknown, sitk::sitkUInt8, -2 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsUInt8();
    pixelSize  = sizeof( uint8_t );
    break;
  case sitk::ConditionalValue< sitk::sitkInt8 != sitk::sitkUnknown, sitk::sitkInt8, -3 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsInt8();
    pixelSize  = sizeof( int8_t );
    break;
  case sitk::ConditionalValue< sitk::sitkUInt16 != sitk::sitkUnknown, sitk::sitkUInt16, -4 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsUInt16();
    pixelSize  = sizeof( uint16_t );
    break;
  case sitk::ConditionalValue< sitk::sitkInt16 != sitk::sitkUnknown, sitk::sitkInt16, -5 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsInt16();
    pixelSize  = sizeof( int16_t );
    break;
  case sitk::ConditionalValue< sitk::sitkUInt32 != sitk::sitkUnknown, sitk::sitkUInt32, -6 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsUInt32();
    pixelSize  = sizeof( uint32_t );
    break;
  case sitk::ConditionalValue< sitk::sitkInt32 != sitk::sitkUnknown, sitk::sitkInt32, -7 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsInt32();
    pixelSize  = sizeof( int32_t );
    break;
  case sitk::ConditionalValue< sitk::sitkUInt64 != sitk::sitkUnknown, sitk::sitkUInt64, -8 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsUInt64();
    pixelSize  = sizeof( uint64_t );
    break;
  case sitk::ConditionalValue< sitk::sitkInt64 != sitk::sitkUnknown, sitk::sitkInt64, -9 >::Value:
     sitkBufferPtr = (const void *)sitkImage->GetBufferAsInt64();
     pixelSize  = sizeof( int64_t );
     break;
  case sitk::ConditionalValue< sitk::sitkFloat32 != sitk::sitkUnknown, sitk::sitkFloat32, -10 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsFloat();
    pixelSize  = sizeof( float );
    break;
  case sitk::ConditionalValue< sitk::sitkFloat64 != sitk::sitkUnknown, sitk::sitkFloat64, -11 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsDouble(); // \todo rename to Float64 for consistency
    pixelSize  = sizeof( double );
    break;
  case sitk::ConditionalValue< sitk::sitkComplexFloat32 != sitk::sitkUnknown, sitk::sitkComplexFloat32, -12 >::Value:
  case sitk::ConditionalValue< sitk::sitkComplexFloat64 != sitk::sitkUnknown, sitk::sitkComplexFloat64, -13 >::Value:
    PyErr_SetString( PyExc_RuntimeError, "Images of Complex Pixel types currently are not supported." );
    SWIG_fail;
    break;
// \todo re-enable when Image class gets more GetBuffer support
  //case sitk::sitkComplexFloat32:
    //pixelDtype = 10;
    //sitkBufferPtr = (const void *)sitkImage->GetBufferAsComplexFloat32();
    //pixelSize  = sizeof( std::complex<float> );
    //break;
  //case sitk::sitkComplexFloat64:
    //pixelDtype = 11;
    //sitkBufferPtr = (const void *)sitkImage->GetBufferAsComplexFloat64();
    //pixelSize  = sizeof( std::complex<double> );
    //break;
  case sitk::ConditionalValue< sitk::sitkVectorUInt8 != sitk::sitkUnknown, sitk::sitkVectorUInt8, -14 >::Value:
  case sitk::ConditionalValue< sitk::sitkVectorInt8 != sitk::sitkUnknown, sitk::sitkVectorInt8, -15 >::Value:
  case sitk::ConditionalValue< sitk::sitkVectorUInt16 != sitk::sitkUnknown, sitk::sitkVectorUInt16, -16 >::Value:
  case sitk::ConditionalValue< sitk::sitkVectorInt16 != sitk::sitkUnknown, sitk::sitkVectorInt16, -17 >::Value:
  case sitk::ConditionalValue< sitk::sitkVectorUInt32 != sitk::sitkUnknown, sitk::sitkVectorUInt32, -18 >::Value:
  case sitk::ConditionalValue< sitk::sitkVectorInt32 != sitk::sitkUnknown, sitk::sitkVectorInt32, -19 >::Value:
  case sitk::ConditionalValue< sitk::sitkVectorUInt64 != sitk::sitkUnknown, sitk::sitkVectorUInt64, -20 >::Value:
  case sitk::ConditionalValue< sitk::sitkVectorInt64 != sitk::sitkUnknown, sitk::sitkVectorInt64, -21 >::Value:
  case sitk::ConditionalValue< sitk::sitkVectorFloat32 != sitk::sitkUnknown, sitk::sitkVectorFloat32, -22 >::Value:
  case sitk::ConditionalValue< sitk::sitkVectorFloat64 != sitk::sitkUnknown, sitk::sitkVectorFloat64, -23 >::Value:
    PyErr_SetString( PyExc_RuntimeError, "Images of Vector Pixel types currently are not supported." );
    SWIG_fail;
    break;
  default:
    PyErr_SetString( PyExc_RuntimeError, "Unknown pixel type." );
    SWIG_fail;
    }

  dimension = sitkImage->GetDimension();
  size = sitkImage->GetSize();
  if( dimension == 2 )
    {
    len = size[0] * size[1] * pixelSize;
    }
  else if( dimension == 3 )
    {
    len = size[0] * size[1] * size[2] * pixelSize;
    }
  else
    {
    PyErr_SetString( PyExc_ValueError, "Unexpected Image dimension." );
    SWIG_fail;
    }
  // When the string is null, the bytearray is uninitialized but allocated
  byteArray = PyByteArray_FromStringAndSize( NULL, len );
  if( !byteArray )
    {
    SWIG_fail;
    }

  // For more information on the Py_buffer protocol, see PEP 3118
  // http://www.python.org/dev/peps/pep-3118/
  // This restricts us to Python 2.6 or better
  Py_buffer byteArrayView;
  if( PyObject_GetBuffer( byteArray, &byteArrayView, PyBUF_WRITABLE ) < 0 )
    {
    SWIG_fail;
    }
  memcpy( byteArrayView.buf, sitkBufferPtr, len );
  PyBuffer_Release( &byteArrayView );

  pixelID = Py_BuildValue( "i", pixelIDValue );
  if( !pixelID )
    {
    SWIG_fail;
    }

  if( dimension == 2 )
    {
    shape = Py_BuildValue( "(ii)", size[1], size[0] );
    }
  else if( dimension == 3 )
    {
    shape = Py_BuildValue( "(iii)", size[2], size[1], size[0] );
    }
  else
    {
    PyErr_SetString( PyExc_ValueError, "Unexpected Image dimension." );
    SWIG_fail;
    }
  if( !shape )
    {
    SWIG_fail;
    }

  resultTuple = PyTuple_New( 3 );
  if( !resultTuple )
    {
    SWIG_fail;
    }
  if( PyTuple_SetItem( resultTuple, 0, byteArray ) != 0 )
    {
    SWIG_fail;
    }
  if( PyTuple_SetItem( resultTuple, 1, shape ) != 0 )
    {
    SWIG_fail;
    }
  if( PyTuple_SetItem( resultTuple, 2, pixelID ) != 0 )
    {
    SWIG_fail;
    }
  return resultTuple;

fail:
  Py_XDECREF( resultTuple ); // this should come first
  Py_XDECREF( shape );
  Py_XDECREF( pixelID );
  Py_XDECREF( byteArray );
  return NULL;
}

static PyObject *
sitk_GetImageFromArray( PyObject *SWIGUNUSEDPARM(self), PyObject *args )
{
  PyObject * arr = NULL;
  PyObject * dtype = NULL;
  PyObject * dtype_num = NULL;
  long dtype_num_long = -1;

  // For more information on the Py_buffer protocol, see PEP 3118
  // http://www.python.org/dev/peps/pep-3118/
  // This restricts us to Python 2.6 or better
  Py_buffer arrayView;
  arrayView.buf = NULL;
  sitk::Image * image = NULL;
  PyObject * pyImage = NULL;
  void * sitkBufferPtr;

  Py_ssize_t *shape = NULL;
  if( !PyArg_ParseTuple( args, "O", &arr ) )
    {
    SWIG_fail; // SWIG_fail is a macro that says goto: fail (return NULL)
    }

  if( PyObject_GetBuffer( arr, &arrayView, PyBUF_CONTIG_RO | PyBUF_C_CONTIGUOUS ) < 0 )
    {
    SWIG_fail;
    }

  dtype = PyObject_GetAttrString( arr, "dtype" );
  if( dtype == NULL )
    {
    SWIG_fail;
    }

  dtype_num = PyObject_GetAttrString( dtype, "num" );
  if( dtype_num == NULL )
    {
    SWIG_fail;
    }
  dtype_num_long = PyInt_AsLong( dtype_num );
  Py_DECREF( dtype_num );
  Py_DECREF( dtype );

  try
    {
    shape = arrayView.shape;
    if( arrayView.ndim == 2 )
      {
      switch( dtype_num_long )
        {
      case 1:
        image = new sitk::Image( shape[1], shape[0], sitk::sitkInt8 );
        sitkBufferPtr = (void *)image->GetBufferAsInt8();
        break;
      case 2:
        image = new sitk::Image( shape[1], shape[0], sitk::sitkUInt8 );
        sitkBufferPtr = (void *)image->GetBufferAsUInt8();
        break;
      case 3:
        image = new sitk::Image( shape[1], shape[0], sitk::sitkInt16 );
        sitkBufferPtr = (void *)image->GetBufferAsInt16();
        break;
      case 4:
        image = new sitk::Image( shape[1], shape[0], sitk::sitkUInt16 );
        sitkBufferPtr = (void *)image->GetBufferAsUInt16();
        break;
      case 5:
        image = new sitk::Image( shape[1], shape[0], sitk::sitkInt32 );
        sitkBufferPtr = (void *)image->GetBufferAsInt32();
        break;
      case 6:
        image = new sitk::Image( shape[1], shape[0], sitk::sitkUInt32 );
        sitkBufferPtr = (void *)image->GetBufferAsUInt32();
        break;
        //! \todo when sitk gets these implemented
      //case 7:
        //image = new sitk::Image( shape[1], shape[0], sitk::sitkInt64 );
        //sitkBufferPtr = (void *)image->GetBufferAsInt64();
        //break;
      //case 8:
        //image = new sitk::Image( shape[1], shape[0], sitk::sitkUInt64 );
        //sitkBufferPtr = (void *)image->GetBufferAsUInt64();
        //break;
      case 11:
        image = new sitk::Image( shape[1], shape[0], sitk::sitkFloat32 );
        sitkBufferPtr = (void *)image->GetBufferAsFloat();
        break;
      case 12:
        image = new sitk::Image( shape[1], shape[0], sitk::sitkFloat64 );
        sitkBufferPtr = (void *)image->GetBufferAsDouble();
        break;
//! \todo re-enable when Image class gets more GetBuffer support
      //case 14:
        //image = new sitk::Image( shape[1], shape[0], sitk::sitkComplexFloat32 );
        //sitkBufferPtr = (void *)image->GetBufferAsComplexFloat();
        //break;
      //case 15:
        //image = new sitk::Image( shape[1], shape[0], sitk::sitkComplexFloat64 );
        //sitkBufferPtr = (void *)image->GetBufferAsComplexDouble();
        //break;
      default:
        PyErr_SetString( PyExc_TypeError, "Unsupported array dtype." );
        SWIG_fail;
        } // end switch dtype_num
      } // end if ndim == 2
    else if( arrayView.ndim == 3 )
      {
      switch( dtype_num_long )
        {
      case 1:
        image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkInt8 );
        sitkBufferPtr = (void *)image->GetBufferAsInt8();
        break;
      case 2:
        image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkUInt8 );
        sitkBufferPtr = (void *)image->GetBufferAsUInt8();
        break;
      case 3:
        image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkInt16 );
        sitkBufferPtr = (void *)image->GetBufferAsInt16();
        break;
      case 4:
        image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkUInt16 );
        sitkBufferPtr = (void *)image->GetBufferAsUInt16();
        break;
      case 5:
        image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkInt32 );
        sitkBufferPtr = (void *)image->GetBufferAsInt32();
        break;
      case 6:
        image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkUInt32 );
        sitkBufferPtr = (void *)image->GetBufferAsUInt32();
        break;
        //! \todo when sitk gets these implemented
      //case 7:
        //image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkInt64 );
        //sitkBufferPtr = (void *)image->GetBufferAsInt64();
        //break;
      //case 8:
        //image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkUInt64 );
        //sitkBufferPtr = (void *)image->GetBufferAsUInt64();
        //break;
      case 11:
        image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkFloat32 );
        sitkBufferPtr = (void *)image->GetBufferAsFloat();
        break;
      case 12:
        image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkFloat64 );
        sitkBufferPtr = (void *)image->GetBufferAsDouble();
        break;
//! \todo re-enable when Image class gets more GetBuffer support
      //case 14:
        //image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkComplexFloat32 );
        //sitkBufferPtr = (void *)image->GetBufferAsComplexFloat();
        //break;
      //case 15:
        //image = new sitk::Image( shape[2], shape[1], shape[0], sitk::sitkComplexFloat64 );
        //sitkBufferPtr = (void *)image->GetBufferAsComplexDouble();
        //break;
      default:
        PyErr_SetString( PyExc_TypeError, "Unsupported array dtype." );
        SWIG_fail;
        } // end switch dtype_num
      } // end if ndim == 3
    else
      {
      PyErr_SetString( PyExc_ValueError, "Unsupported array dimension." );
      SWIG_fail;
      }
    }
  catch( const std::exception &e )
    {
    std::string msg = "Exception thrown in SimpleITK new Image: ";
    msg += e.what();
    PyErr_SetString( PyExc_RuntimeError, msg.c_str() );
    SWIG_fail;
    }

  pyImage = SWIG_NewPointerObj( image, SWIGTYPE_p_itk__simple__Image, 1 );
  memcpy( (void *)sitkBufferPtr, arrayView.buf, arrayView.len );

  PyBuffer_Release( &arrayView );

  return pyImage;

fail:
  Py_XDECREF( dtype );
  Py_XDECREF( pyImage );
  if( arrayView.buf != NULL )
    {
    PyBuffer_Release( &arrayView );
    }
  if( image != NULL )
    {
    delete image;
    }
  return NULL;
}

#ifdef __cplusplus
}
#endif

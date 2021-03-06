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
#ifndef __sitkDetail_h
#define __sitkDetail_h

#include "sitkMemberFunctionFactoryBase.h"
#include "Ancillary/FunctionTraits.h"

namespace itk {
namespace simple {

// this namespace is internal classes not part of the external simple ITK interface
namespace detail {


template < class TMemberFunctionPointer >
struct MemberFunctionAddressor
{
  typedef typename ::detail::FunctionTraits<TMemberFunctionPointer>::ClassType ObjectType;

  template< typename TImageType >
  TMemberFunctionPointer operator() ( void ) const
    {
      return &ObjectType::template ExecuteInternal< TImageType >;
    }
};

template < class TMemberFunctionPointer >
struct DualExecuteInternalAddressor
{
  typedef typename ::detail::FunctionTraits<TMemberFunctionPointer>::ClassType ObjectType;

  template< typename TImageType1, typename TImageType2 >
  TMemberFunctionPointer operator() ( void ) const
    {
      return &ObjectType::template DualExecuteInternal< TImageType1, TImageType2 >;
    }
};

/** An addressor of ExecuteInternalCast to be utilized with
 * registering member functions with the factory.
 */
template < class TMemberFunctionPointer >
struct ExecuteInternalVectorImageAddressor
{
  typedef typename ::detail::FunctionTraits<TMemberFunctionPointer>::ClassType ObjectType;

  template< typename TImageType >
  TMemberFunctionPointer operator() ( void ) const
  {
    return &ObjectType::template ExecuteInternalVectorImage< TImageType >;
  }
};


}
}
}
#endif

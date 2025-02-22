/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkNumericTraitsVectorPixel_h
#define itkNumericTraitsVectorPixel_h

#include "itkNumericTraits.h"
#include "itkVector.h"

namespace itk
{
/** \brief NumericTraits for Vector
 * \tparam T Component type for Vector
 * \tparam D Space dimension (Dimension of Vector)
 */
template <typename T, unsigned int D>
class NumericTraits<Vector<T, D>>
{
private:
  using ElementAbsType = typename NumericTraits<T>::AbsType;
  using ElementAccumulateType = typename NumericTraits<T>::AccumulateType;
  using ElementFloatType = typename NumericTraits<T>::FloatType;
  using ElementPrintType = typename NumericTraits<T>::PrintType;
  using ElementRealType = typename NumericTraits<T>::RealType;

public:
  /** Return the type of the native component type. */
  using ValueType = T;
  using Self = Vector<T, D>;

  /** Unsigned component type */
  using AbsType = Vector<ElementAbsType, D>;

  /** Accumulation of addition and multiplication. */
  using AccumulateType = Vector<ElementAccumulateType, D>;

  /** Typedef for operations that use floating point instead of real precision
   */
  using FloatType = Vector<ElementFloatType, D>;

  /** Return the type that can be printed. */
  using PrintType = Vector<ElementPrintType, D>;

  /** Type for real-valued scalar operations. */
  using RealType = Vector<ElementRealType, D>;

  /** Type for real-valued scalar operations. */
  using ScalarRealType = ElementRealType;

  /** Measurement vector type */
  using MeasurementVectorType = Self;

  /** Component wise defined element
   *
   * \note minimum value for floating pointer types is defined as
   * minimum positive normalize value.
   */
  static const Self
  max(const Self &)
  {
    return MakeFilled<Self>(NumericTraits<T>::max());
  }

  static const Self
  min(const Self &)
  {
    return MakeFilled<Self>(NumericTraits<T>::min());
  }

  static const Self
  max()
  {
    return MakeFilled<Self>(NumericTraits<T>::max());
  }

  static const Self
  min()
  {
    return MakeFilled<Self>(NumericTraits<T>::min());
  }

  static const Self
  NonpositiveMin()
  {
    return MakeFilled<Self>(NumericTraits<T>::NonpositiveMin());
  }

  static const Self
  ZeroValue()
  {
    return MakeFilled<Self>(NumericTraits<T>::ZeroValue());
  }

  static const Self
  OneValue()
  {
    return MakeFilled<Self>(NumericTraits<T>::OneValue());
  }

  static const Self
  NonpositiveMin(const Self &)
  {
    return NonpositiveMin();
  }

  static const Self
  ZeroValue(const Self &)
  {
    return ZeroValue();
  }

  static const Self
  OneValue(const Self &)
  {
    return OneValue();
  }

  static bool
  IsPositive(const Self & a)
  {
    bool flag = false;
    for (unsigned int i = 0; i < GetLength(a); ++i)
    {
      if (a[i] > NumericTraits<ValueType>::ZeroValue())
      {
        flag = true;
      }
    }
    return flag;
  }

  static bool
  IsNonpositive(const Self & a)
  {
    bool flag = false;
    for (unsigned int i = 0; i < GetLength(a); ++i)
    {
      if (!(a[i] > 0.0))
      {
        flag = true;
      }
    }
    return flag;
  }

  static bool
  IsNegative(const Self & a)
  {
    bool flag = false;
    for (unsigned int i = 0; i < GetLength(a); ++i)
    {
      if (a[i] < 0.0)
      {
        flag = true;
      }
    }
    return flag;
  }

  static bool
  IsNonnegative(const Self & a)
  {
    bool flag = false;
    for (unsigned int i = 0; i < GetLength(a); ++i)
    {
      if (!(a[i] < 0.0))
      {
        flag = true;
      }
    }
    return flag;
  }

  static constexpr bool IsSigned = std::is_signed_v<ValueType>;
  static constexpr bool IsInteger = std::is_integral_v<ValueType>;
  static constexpr bool IsComplex = NumericTraits<ValueType>::IsComplex;

  /** Fixed length vectors cannot be resized, so an exception will
   *  be thrown if the input size is not valid.  If the size is valid
   *  the vector will be filled with zeros. */
  static void
  SetLength(Vector<T, D> & m, const unsigned int s)
  {
    if (s != D)
    {
      itkGenericExceptionMacro(<< "Cannot set the size of a Vector of length " << D << " to " << s);
    }
    m.Fill(NumericTraits<T>::ZeroValue());
  }

  /** Return the size of the vector. */
  static unsigned int
  GetLength(const Vector<T, D> &)
  {
    return D;
  }

  /** Return the size of the vector. */
  static unsigned int
  GetLength()
  {
    return D;
  }

  static void
  AssignToArray(const Self & v, MeasurementVectorType & mv)
  {
    mv = v;
  }

  template <typename TArray>
  static void
  AssignToArray(const Self & v, TArray & mv)
  {
    for (unsigned int i = 0; i < D; ++i)
    {
      mv[i] = v[i];
    }
  }

  /** \note: the functions are preferred over the member variables as
   * they are defined for all partial specialization
   */
  static const Self ITKCommon_EXPORT Zero;
  static const Self ITKCommon_EXPORT One;
};
} // end namespace itk

#endif // itkNumericTraitsVectorPixel_h

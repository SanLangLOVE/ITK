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
#ifndef itkTransform_hxx
#define itkTransform_hxx

#include "itkCrossHelper.h"
#include "vnl/algo/vnl_svd_fixed.h"
#include "itkMetaProgrammingLibrary.h"

namespace itk
{

template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
Transform<TParametersValueType, VInputDimension, VOutputDimension>::Transform(NumberOfParametersType numberOfParameters)
  : m_Parameters(numberOfParameters)
  , m_FixedParameters()
{}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
std::string
Transform<TParametersValueType, VInputDimension, VOutputDimension>::GetTransformTypeAsString() const
{
  std::ostringstream n;

  n << GetNameOfClass() << '_' << this->GetTransformTypeAsString(static_cast<TParametersValueType *>(nullptr)) << '_'
    << this->GetInputSpaceDimension() << '_' << this->GetOutputSpaceDimension();
  return n.str();
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
typename LightObject::Pointer
Transform<TParametersValueType, VInputDimension, VOutputDimension>::InternalClone() const
{
  // Default implementation just copies the parameters from
  // this to new transform.
  typename LightObject::Pointer loPtr = Superclass::InternalClone();

  typename Self::Pointer rval = dynamic_cast<Self *>(loPtr.GetPointer());
  if (rval.IsNull())
  {
    itkExceptionMacro(<< "downcast to type " << this->GetNameOfClass() << " failed.");
  }
  rval->SetFixedParameters(this->GetFixedParameters());
  rval->SetParameters(this->GetParameters());
  return loPtr;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
void
Transform<TParametersValueType, VInputDimension, VOutputDimension>::UpdateTransformParameters(
  const DerivativeType & update,
  ParametersValueType    factor)
{
  NumberOfParametersType numberOfParameters = this->GetNumberOfParameters();

  if (update.Size() != numberOfParameters)
  {
    itkExceptionMacro("Parameter update size, " << update.Size()
                                                << ", must "
                                                   " be same as transform parameter size, "
                                                << numberOfParameters << std::endl);
  }

  /* Make sure m_Parameters is updated to reflect the current values in
   * the transform's other parameter-related variables. This is effective for
   * managing the parallel variables used for storing parameter data,
   * but inefficient. However for small global transforms, shouldn't be
   * too bad. Dense-field transform will want to make sure m_Parameters
   * is always updated whenever the transform is changed, so GetParameters
   * can be skipped in their implementations of UpdateTransformParameters. */
  this->GetParameters();

  if (factor == 1.0)
  {
    for (NumberOfParametersType k = 0; k < numberOfParameters; ++k)
    {
      this->m_Parameters[k] += update[k];
    }
  }
  else
  {
    for (NumberOfParametersType k = 0; k < numberOfParameters; ++k)
    {
      this->m_Parameters[k] += update[k] * factor;
    }
  }

  /* Call SetParameters with the updated parameters.
   * SetParameters in most transforms is used to assign the input params
   * to member variables, possibly with some processing. The member variables
   * are then used in TransformPoint.
   * In the case of dense-field transforms that are updated in blocks from
   * a threaded implementation, SetParameters doesn't do this, and is
   * optimized to not copy the input parameters when == m_Parameters.
   */
  this->SetParameters(this->m_Parameters);

  /* Call Modified, following behavior of other transform when their
   * parameters change, e.g. MatrixOffsetTransformBase */
  this->Modified();
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
auto
Transform<TParametersValueType, VInputDimension, VOutputDimension>::TransformVector(const InputVectorType & vector,
                                                                                    const InputPointType &  point) const
  -> OutputVectorType
{
  JacobianPositionType jacobian;
  this->ComputeJacobianWithRespectToPosition(point, jacobian);
  OutputVectorType result;
  for (unsigned int i = 0; i < VOutputDimension; ++i)
  {
    result[i] = NumericTraits<TParametersValueType>::ZeroValue();
    for (unsigned int j = 0; j < VInputDimension; ++j)
    {
      result[i] += jacobian[i][j] * vector[j];
    }
  }

  return result;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
auto
Transform<TParametersValueType, VInputDimension, VOutputDimension>::TransformVector(const InputVnlVectorType & vector,
                                                                                    const InputPointType & point) const
  -> OutputVnlVectorType
{
  JacobianPositionType jacobian;
  this->ComputeJacobianWithRespectToPosition(point, jacobian);
  OutputVnlVectorType result;
  for (unsigned int i = 0; i < VOutputDimension; ++i)
  {
    result[i] = NumericTraits<ParametersValueType>::ZeroValue();
    for (unsigned int j = 0; j < VInputDimension; ++j)
    {
      result[i] += jacobian[i][j] * vector[j];
    }
  }

  return result;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
auto
Transform<TParametersValueType, VInputDimension, VOutputDimension>::TransformVector(const InputVectorPixelType & vector,
                                                                                    const InputPointType & point) const
  -> OutputVectorPixelType
{

  if (vector.GetSize() != VInputDimension)
  {
    itkExceptionMacro("Input Vector is not of size VInputDimension = " << VInputDimension << std::endl);
  }

  JacobianPositionType jacobian;
  this->ComputeJacobianWithRespectToPosition(point, jacobian);

  OutputVectorPixelType result;
  result.SetSize(VOutputDimension);

  for (unsigned int i = 0; i < VOutputDimension; ++i)
  {
    result[i] = NumericTraits<ParametersValueType>::ZeroValue();
    for (unsigned int j = 0; j < VInputDimension; ++j)
    {
      result[i] += jacobian[i][j] * vector[j];
    }
  }

  return result;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
auto
Transform<TParametersValueType, VInputDimension, VOutputDimension>::TransformCovariantVector(
  const InputCovariantVectorType & vector,
  const InputPointType &           point) const -> OutputCovariantVectorType
{
  InverseJacobianPositionType jacobian;
  this->ComputeInverseJacobianWithRespectToPosition(point, jacobian);
  OutputCovariantVectorType result;
  for (unsigned int i = 0; i < VOutputDimension; ++i)
  {
    result[i] = NumericTraits<TParametersValueType>::ZeroValue();
    for (unsigned int j = 0; j < VInputDimension; ++j)
    {
      result[i] += jacobian[j][i] * vector[j];
    }
  }

  return result;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
auto
Transform<TParametersValueType, VInputDimension, VOutputDimension>::TransformCovariantVector(
  const InputVectorPixelType & vector,
  const InputPointType &       point) const -> OutputVectorPixelType
{

  if (vector.GetSize() != VInputDimension)
  {
    itkExceptionMacro("Input Vector is not of size VInputDimension = " << VInputDimension << std::endl);
  }

  InverseJacobianPositionType jacobian;
  this->ComputeInverseJacobianWithRespectToPosition(point, jacobian);

  OutputVectorPixelType result;
  result.SetSize(VOutputDimension);

  for (unsigned int i = 0; i < VOutputDimension; ++i)
  {
    result[i] = NumericTraits<ParametersValueType>::ZeroValue();
    for (unsigned int j = 0; j < VInputDimension; ++j)
    {
      result[i] += jacobian[j][i] * vector[j];
    }
  }

  return result;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
auto
Transform<TParametersValueType, VInputDimension, VOutputDimension>::TransformDiffusionTensor3D(
  const InputDiffusionTensor3DType & inputTensor,
  const InputPointType &             point) const -> OutputDiffusionTensor3DType
{
  InverseJacobianPositionType invJacobian;
  this->ComputeInverseJacobianWithRespectToPosition(point, invJacobian);

  OutputDiffusionTensor3DType result =
    this->PreservationOfPrincipalDirectionDiffusionTensor3DReorientation(inputTensor, invJacobian);

  return result;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
auto
Transform<TParametersValueType, VInputDimension, VOutputDimension>::TransformDiffusionTensor3D(
  const InputVectorPixelType & inputTensor,
  const InputPointType &       point) const -> OutputVectorPixelType
{
  if (inputTensor.GetSize() != 6)
  {
    itkExceptionMacro("Input DiffusionTensor3D does not have 6 elements" << std::endl);
  }

  InputDiffusionTensor3DType inTensor;
  for (unsigned int i = 0; i < 5; ++i)
  {
    inTensor[i] = inputTensor[i];
  }

  OutputDiffusionTensor3DType outTensor = this->TransformDiffusionTensor3D(inTensor, point);

  OutputVectorPixelType outputTensor;
  outputTensor.SetSize(6);
  for (unsigned int i = 0; i < 5; ++i)
  {
    outputTensor[i] = outTensor[i];
  }

  return outputTensor;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
typename Transform<TParametersValueType, VInputDimension, VOutputDimension>::OutputDiffusionTensor3DType
Transform<TParametersValueType, VInputDimension, VOutputDimension>::
  PreservationOfPrincipalDirectionDiffusionTensor3DReorientation(const InputDiffusionTensor3DType &  inputTensor,
                                                                 const InverseJacobianPositionType & jacobian) const
{
  Matrix<TParametersValueType, 3, 3> matrix;

  matrix.Fill(0.0);
  for (unsigned int i = 0; i < 3; ++i)
  {
    matrix(i, i) = 1.0;
  }

  for (unsigned int i = 0; i < VInputDimension; ++i)
  {
    for (unsigned int j = 0; j < VOutputDimension; ++j)
    {
      if ((i < 3) && (j < 3))
      {
        matrix(i, j) = jacobian(i, j);
      }
    }
  }

  typename InputDiffusionTensor3DType::EigenValuesArrayType   eigenValues;
  typename InputDiffusionTensor3DType::EigenVectorsMatrixType eigenVectors;
  inputTensor.ComputeEigenAnalysis(eigenValues, eigenVectors);

  Vector<TParametersValueType, 3> ev1;
  Vector<TParametersValueType, 3> ev2;
  Vector<TParametersValueType, 3> ev3;
  for (unsigned int i = 0; i < 3; ++i)
  {
    ev1[i] = eigenVectors(2, i);
    ev2[i] = eigenVectors(1, i);
  }

  // Account for image direction changes between moving and fixed spaces
  ev1 = matrix * ev1;
  ev1.Normalize();

  // Get aspect of rotated e2 that is perpendicular to rotated e1
  ev2 = matrix * ev2;
  double dp = ev2 * ev1;
  if (dp < 0)
  {
    ev2 = ev2 * (-1.0);
    dp = dp * (-1.0);
  }
  ev2 = ev2 - ev1 * dp;
  ev2.Normalize();

  CrossHelper<Vector<TParametersValueType, 3>> vectorCross;
  ev3 = vectorCross(ev1, ev2);

  // Outer product matrices
  Matrix<TParametersValueType, 3, 3> e1;
  Matrix<TParametersValueType, 3, 3> e2;
  Matrix<TParametersValueType, 3, 3> e3;
  for (unsigned int i = 0; i < 3; ++i)
  {
    for (unsigned int j = 0; j < 3; ++j)
    {
      e1(i, j) = eigenValues[2] * ev1[i] * ev1[j];
      e2(i, j) = eigenValues[1] * ev2[i] * ev2[j];
      e3(i, j) = eigenValues[0] * ev3[i] * ev3[j];
    }
  }

  Matrix<TParametersValueType, 3, 3> rotated = e1 + e2 + e3;

  OutputDiffusionTensor3DType result; // Converted vector
  result[0] = rotated(0, 0);
  result[1] = rotated(0, 1);
  result[2] = rotated(0, 2);
  result[3] = rotated(1, 1);
  result[4] = rotated(1, 2);
  result[5] = rotated(2, 2);

  return result;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
auto
Transform<TParametersValueType, VInputDimension, VOutputDimension>::TransformSymmetricSecondRankTensor(
  const InputSymmetricSecondRankTensorType & inputTensor,
  const InputPointType &                     point) const -> OutputSymmetricSecondRankTensorType
{
  JacobianPositionType jacobian;
  this->ComputeJacobianWithRespectToPosition(point, jacobian);
  InverseJacobianPositionType invJacobian;
  this->ComputeInverseJacobianWithRespectToPosition(point, invJacobian);
  JacobianType tensor;
  tensor.SetSize(VInputDimension, VInputDimension);

  for (unsigned int i = 0; i < VInputDimension; ++i)
  {
    for (unsigned int j = 0; j < VInputDimension; ++j)
    {
      tensor(i, j) = inputTensor(i, j);
    }
  }

  JacobianType                        outTensor = jacobian * tensor * invJacobian;
  OutputSymmetricSecondRankTensorType outputTensor;

  for (unsigned int i = 0; i < VOutputDimension; ++i)
  {
    for (unsigned int j = 0; j < VOutputDimension; ++j)
    {
      outputTensor(i, j) = outTensor(i, j);
    }
  }

  return outputTensor;
}


template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
auto
Transform<TParametersValueType, VInputDimension, VOutputDimension>::TransformSymmetricSecondRankTensor(
  const InputVectorPixelType & inputTensor,
  const InputPointType &       point) const -> OutputVectorPixelType
{

  if (inputTensor.GetSize() != (VInputDimension * VInputDimension))
  {
    itkExceptionMacro("Input DiffusionTensor3D does not have " << VInputDimension * VInputDimension << " elements"
                                                               << std::endl);
  }

  JacobianPositionType jacobian;
  this->ComputeJacobianWithRespectToPosition(point, jacobian);
  InverseJacobianPositionType invJacobian;
  this->ComputeInverseJacobianWithRespectToPosition(point, invJacobian);
  JacobianType tensor;
  tensor.SetSize(VInputDimension, VInputDimension);

  for (unsigned int i = 0; i < VInputDimension; ++i)
  {
    for (unsigned int j = 0; j < VInputDimension; ++j)
    {
      tensor(i, j) = inputTensor[j + VInputDimension * i];
    }
  }

  JacobianType outTensor = jacobian * tensor * invJacobian;

  OutputVectorPixelType outputTensor;
  outputTensor.SetSize(VOutputDimension * VOutputDimension);

  for (unsigned int i = 0; i < VOutputDimension; ++i)
  {
    for (unsigned int j = 0; j < VOutputDimension; ++j)
    {
      outputTensor[j + VOutputDimension * i] = outTensor(i, j);
    }
  }

  return outputTensor;
}

template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
template <typename TImage>
std::enable_if_t<TImage::ImageDimension == VInputDimension && TImage::ImageDimension == VOutputDimension, void>
Transform<TParametersValueType, VInputDimension, VOutputDimension>::ApplyToImageMetadata(TImage * image) const
{
  using ImageType = TImage;

  if (!this->IsLinear())
  {
    itkWarningMacro(<< "ApplyToImageMetadata was invoked with non-linear transform of type: " << this->GetNameOfClass()
                    << ". This might produce unexpected results.");
  }

  typename Self::Pointer inverse = this->GetInverseTransform();

  // transform origin
  typename ImageType::PointType origin = image->GetOrigin();
  origin = inverse->TransformPoint(origin);
  image->SetOrigin(origin);

  typename ImageType::SpacingType   spacing = image->GetSpacing();
  typename ImageType::DirectionType direction = image->GetDirection();
  // transform direction cosines and compute new spacing
  for (unsigned int i = 0; i < ImageType::ImageDimension; ++i)
  {
    Vector<typename Self::ParametersValueType, ImageType::ImageDimension> dirVector;
    for (unsigned int k = 0; k < ImageType::ImageDimension; ++k)
    {
      dirVector[k] = direction[k][i];
    }

    dirVector *= spacing[i];
    dirVector = inverse->TransformVector(dirVector);
    spacing[i] = dirVector.Normalize();

    for (unsigned int k = 0; k < ImageType::ImageDimension; ++k)
    {
      direction[k][i] = dirVector[k];
    }
  }
  image->SetDirection(direction);
  image->SetSpacing(spacing);
}


#if !defined(ITK_LEGACY_REMOVE)
template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
void
Transform<TParametersValueType, VInputDimension, VOutputDimension>::ComputeJacobianWithRespectToPosition(
  const InputPointType & pnt,
  JacobianType &         jacobian) const
{
  JacobianPositionType jacobian_fixed;
  this->ComputeJacobianWithRespectToPosition(pnt, jacobian_fixed);
  jacobian.SetSize(VOutputDimension, VInputDimension);
  jacobian.set(jacobian_fixed.data_block());
}

template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
void
Transform<TParametersValueType, VInputDimension, VOutputDimension>::ComputeInverseJacobianWithRespectToPosition(
  const InputPointType & pnt,
  JacobianType &         jacobian) const
{
  InverseJacobianPositionType jacobian_fixed;
  this->ComputeInverseJacobianWithRespectToPosition(pnt, jacobian_fixed);
  jacobian.SetSize(VInputDimension, VOutputDimension);
  jacobian.set(jacobian_fixed.data_block());
}
#endif

template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
void
Transform<TParametersValueType, VInputDimension, VOutputDimension>::ComputeInverseJacobianWithRespectToPosition(
  const InputPointType &        pnt,
  InverseJacobianPositionType & jacobian) const
{
  JacobianPositionType forward_jacobian;
  this->ComputeJacobianWithRespectToPosition(pnt, forward_jacobian);

  using SVDAlgorithmType =
    vnl_svd_fixed<typename JacobianPositionType::element_type, VOutputDimension, VInputDimension>;
  SVDAlgorithmType svd(forward_jacobian);
  jacobian.set(svd.inverse().data_block());
}

template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
void
Transform<TParametersValueType, VInputDimension, VOutputDimension>::CopyInParameters(
  const ParametersValueType * const begin,
  const ParametersValueType * const end)
{
  if (begin == end)
  {
    return;
  }
  // Ensure that we are not copying onto self
  if (begin != &(this->m_Parameters[0]))
  {
    // Copy raw values array
    std::copy(begin, end, this->m_Parameters.data_block());
  }
  // Now call child class set parameter to interpret raw values
  this->SetParameters(this->m_Parameters);
}

template <typename TParametersValueType, unsigned int VInputDimension, unsigned int VOutputDimension>
void
Transform<TParametersValueType, VInputDimension, VOutputDimension>::CopyInFixedParameters(
  const FixedParametersValueType * const begin,
  const FixedParametersValueType * const end)
{
  if (begin == end)
  {
    return;
  }
  // Ensure that we are not copying onto self
  if (begin != &(this->m_FixedParameters[0]))
  {
    // Copy raw values array
    std::copy(begin, end, this->m_FixedParameters.data_block());
  }
  // Now call child class set parameter to interpret raw values
  this->SetFixedParameters(this->m_FixedParameters);
}

} // end namespace itk

#endif

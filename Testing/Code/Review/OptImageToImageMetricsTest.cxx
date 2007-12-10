
#include "itkImage.h"
#include "itkTranslationTransform.h"
#include "itkAffineTransform.h"
#include "itkRigid2DTransform.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkLinearInterpolateImageFunction.h"

#include "itkMeanSquaresImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkMutualInformationImageToImageMetric.h"
#include "itkMersenneTwisterRandomVariateGenerator.h"

#include "OptImageToImageMetricsTest.h"

int main(int argc, char* argv[])
{
#ifdef ITK_USE_OPTIMIZED_REGISTRATION_METHODS
  std::cout << "OPTIMIZED ON" << std::endl;
#else
  std::cout << "OPTIMIZED OFF" << std::endl;  
#endif

  std::cout << "Default number of threads : " 
            << itk::MultiThreader::GetGlobalDefaultNumberOfThreads() 
            << std::endl;

  typedef itk::Image< unsigned int > FixedImageType;
  typedef itk::Image< unsigned int > MovingImageType;
  typedef itk::ImageFileReader< FixedImageType  > FixedImageReaderType;
  typedef itk::ImageFileReader< MovingImageType > MovingImageReaderType;
  FixedImageReaderType::Pointer  fixedImageReader  = FixedImageReaderType::New();
  MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

  fixedImageReader->SetFileName(  argv[1] );
  movingImageReader->SetFileName( argv[2] );

  TranslationLinearTest( fixedImageReader.GetPointer(), 
                    movingImageReader.GetPointer() );

  RigidLinearTest( fixedImageReader.GetPointer(), 
                    movingImageReader.GetPointer() );

  AffineLinearTest( fixedImageReader.GetPointer(), 
                    movingImageReader.GetPointer() );

#ifdef ITK_USE_OPTIMIZED_REGISTRATION_METHODS
  std::cout << "OPTIMIZED ON" << std::endl;
#else
  std::cout << "OPTIMIZED OFF" << std::endl;  
#endif

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // DO EXPERIMENTS WITH NUMBER OF THREADS SET TO 1!!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
  std::cout << "Running tests with : " << std::endl;
  std::cout << "\t itk::MultiThreader::SetGlobalDefaultNumberOfThreads(1); " << std::endl;
  std::cout << "\t itk::MultiThreader::SetGlobalMaximumNumberOfThreads(1); " << std::endl;
  std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
  std::cout << std::endl;

  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(1);
  itk::MultiThreader::SetGlobalMaximumNumberOfThreads(1);

  TranslationLinearTest( fixedImageReader.GetPointer(), 
                    movingImageReader.GetPointer() );

  RigidLinearTest( fixedImageReader.GetPointer(), 
                    movingImageReader.GetPointer() );

  AffineLinearTest( fixedImageReader.GetPointer(), 
                    movingImageReader.GetPointer() );

#ifdef ITK_USE_OPTIMIZED_REGISTRATION_METHODS
  std::cout << "OPTIMIZED ON" << std::endl;
#else
  std::cout << "OPTIMIZED OFF" << std::endl;  
#endif

  // For right now, make this test always fail so that we can gather
  // some performance information from the platforms running the new
  // optimized code.
  return EXIT_FAILURE;
}

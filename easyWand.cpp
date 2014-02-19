#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <vtkChartXY.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkFloatArray.h>
#include <vtkPlotPoints.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTable.h>
#include <Eigen/Dense>
#include "easyWand.h"

int gv_DEBUG = 0;

// EasyWand method definitions
// ================================================================================
easyWand::easyWand(const std::string camProfile, const std::string wandPtsFile, const std::string bkgdPtsFile){
    if(camProfile == ""){           // Check that a camera profile was given
    std::cerr << "No camera profile specified!\n";
    }
    // If we got a camera profiles file, then we should read that in
    readCameraProfiles(camProfile);
    if( gv_DEBUG){                  // Now we see if that worked...
        std::cout << "\n Number of cameras? \n";
        std::cout << numCameras << "\n";
        std::cout << "\n Camera resolutions? \n";
        std::copy(resolutions.begin(), resolutions.end(), std::ostream_iterator<double>(std::cout, " "));
        std::cout << "\n Camera focal lengths? \n";
        std::copy(foc.begin(), foc.end(), std::ostream_iterator<double>(std::cout, " "));
    }

    if(wandPtsFile == ""){          // Check that a wand point file was given
        std::cerr << "No wand point file specified!\n";
    }
    // If we have a wand points file, then read it in and get the appropriate number of cameras from that
    readWandPoints(wandPtsFile, wandPts);
    if( gv_DEBUG){                  // Let's check if it worked!
        std::cout << " Wand points? \n";
        for(int ii = 0; ii < wandPts.size(); ii++){
            std::copy(wandPts[ii].begin(), wandPts[ii].end(), std::ostream_iterator<double>(std::cout, " "));
            std::cout << "\n";
        }
    }

    if(bkgdPtsFile == ""){
        std::cout << "\nNo background point file specified, continuing anyways.... \n";
    }
    else{
    // Read in the background points same as the wand points.
    readWandPoints(bkgdPtsFile, bkgdPts);
    }
    if(gv_DEBUG){
        std::cout << " Background points? \n";
        for(int ii=0; ii < bkgdPts.size(); ii ++){
            std::copy(bkgdPts[ii].begin(), bkgdPts[ii].end(), std::ostream_iterator<double>(std::cout, " "));
            std::cout << "\n";
        }
    }

    // Everything should be set up for the calibration now...
}


void easyWand::readCameraProfiles(const std::string camProfile){
    ppts.clear();
    resolutions.clear();
    foc.clear();
    numCameras = 0;
    std::string aline;              // Put each line in here
    std::ifstream fin(camProfile);  // Set up the file stream
    while(std::getline(fin,aline)){
        std::stringstream linestream(aline);
        int xRes;
        int yRes;
        double xPpt; 
        double yPpt;
        double fl;
        int camNumber;
        linestream >> camNumber >> fl >> xRes >> yRes >> xPpt >> yPpt;
        ppts.push_back(xPpt);
        ppts.push_back(yPpt);
        resolutions.push_back(xRes);
        resolutions.push_back(yRes);
        foc.push_back(fl);
        numCameras++;
    }
}

void easyWand::readWandPoints(const std::string wandPtsFile, std::vector<std::vector<double> > &wandPts){
    wandPts.clear();                // Make sure we only get the new wand pts
    std::ifstream fin(wandPtsFile); // Set up the file stream
    std::string line;
    while(std::getline(fin, line)){
        std::istringstream linestream(line);
        std::vector<double> linerow; 
        double token;
        while(linestream >> token){
            linerow.push_back(token);
            if(linestream.peek() == ','){
                linestream.ignore();
            }
        }
        wandPts.push_back(linerow);
    }
} 

void easyWand::plotWandPoints(const std::vector< std::vector<double> > wandPts, const int camera){
    // See if we can't get some visuals up in here
    vtkSmartPointer<vtkContextView> view = 
    vtkSmartPointer<vtkContextView>::New();
    view->GetRenderer()->SetBackground(0.9,0.9,0.9);
    view->GetRenderWindow()->SetSize(600,600);

    vtkSmartPointer<vtkChartXY> chart = 
    vtkSmartPointer<vtkChartXY>::New();
    view->GetScene()->AddItem(chart);
    chart->SetShowLegend(false);

    // Put the wand points in a table to be viewed
    vtkSmartPointer<vtkTable> table = 
    vtkSmartPointer<vtkTable>::New();
    vtkSmartPointer<vtkFloatArray> wand1X =
    vtkSmartPointer<vtkFloatArray>::New();
    wand1X->SetName("Wand1X");
    table->AddColumn(wand1X);

    vtkSmartPointer<vtkFloatArray> wand1Y = 
    vtkSmartPointer<vtkFloatArray>::New();
    wand1Y->SetName("Wand1Y");
    table->AddColumn(wand1Y);

    vtkSmartPointer<vtkFloatArray> wand2X = 
    vtkSmartPointer<vtkFloatArray>::New();
    wand2X->SetName("Wand2X");
    table->AddColumn(wand2X);

    vtkSmartPointer<vtkFloatArray> wand2Y = 
    vtkSmartPointer<vtkFloatArray>::New();
    wand2Y->SetName("Wand2Y");
    table->AddColumn(wand2Y);

    // Fill the table with wand points
    int numPoints = wandPts.size();
    table->SetNumberOfRows(numPoints);
    for(int ii = 0; ii < numPoints; ii++){
        table->SetValue(ii,0,wandPts[ii][(camera-1)*2]);
        table->SetValue(ii,1,wandPts[ii][(camera-1)*2+1]);
        table->SetValue(ii,2,wandPts[ii][(camera-1)*2+numCameras*2]);
        table->SetValue(ii,3,wandPts[ii][(camera-1)*2+1+numCameras*2]);
    }

    // Add the scatter plots, set different colors for wand points
    // Left wand points
    vtkPlot *points = chart->AddPlot(vtkChart::POINTS);
    points->SetInputData(table, 0, 1);
    points->SetColor(100,100,100,255);
    points->SetWidth(1.0);
    vtkPlotPoints::SafeDownCast(points)->SetMarkerStyle(vtkPlotPoints::CIRCLE);
    // Right wand points
    points = chart->AddPlot(vtkChart::POINTS);
    points->SetInputData(table, 2, 3);
    points->SetColor(255,255,255,255);
    points->SetWidth(1.0);
    vtkPlotPoints::SafeDownCast(points)->SetMarkerStyle(vtkPlotPoints::CIRCLE);

    //Finally see what's been plotted
    view->GetRenderWindow()->SetMultiSamples(0);
    view->GetInteractor()->Initialize();
    view->GetInteractor()->Start();
}

void easyWand::computeCalibration(){
    // There should be a function somewhere that appropriately deals with any incompletely digitized points (only visible in two cameras, say)
    // That'll have to come later - for now only testing with complete data

    int nCams = numCameras;
    int nPoints = wandPts.size();
    Eigen::MatrixXd ptMatEig = VectToEigenMat(wandPts);
    Eigen::MatrixXd ptMat2Eig = VectToEigenMat(bkgdPts);
    Eigen::MatrixXd pptsEig = VectToEigenMat(ppts);
    Eigen::MatrixXd flengthsEig = VectToEigenMat(foc);

    // Subtract away the principal point (careful normalization is the best kind of normalization >.>)
    pptsEig.transposeInPlace();
    ptMatEig = ptMatEig - pptsEig.replicate(nPoints,2);

    // Divide by the focal lengths for more normalization
    for(int ii=0; ii < nCams; ii++){
        for(int jj=0; jj < nPoints; jj++){
            ptMatEig(jj, 2*ii) /= foc[ii];
            ptMatEig(jj, 2*ii+1) /= foc[ii];
            ptMatEig(jj, 2*ii+2*nCams) /= foc[ii];
            ptMatEig(jj, 2*ii+1+2*nCams) /= foc[ii];
        }
    }

    // Use eight-point algorithm to get estimate of camera positions and rotations
    // twoCamCal testing
    Eigen::MatrixXd ptNorm = VectToEigenMat(wandPts);
    Eigen::MatrixXd testPtNorm(2*nPoints,4);
    for(int ii=0; ii < nPoints; ii++){
        testPtNorm(ii,0) = ptNorm(ii,0);
        testPtNorm(ii,1) = ptNorm(ii,1);
        testPtNorm(ii,2) = ptNorm(ii,2);
        testPtNorm(ii,3) = ptNorm(ii,3);
        testPtNorm(ii+nPoints,0) = ptNorm(ii,6);
        testPtNorm(ii+nPoints,1) = ptNorm(ii,7);
        testPtNorm(ii+nPoints,2) = ptNorm(ii,8);
        testPtNorm(ii+nPoints,3) = ptNorm(ii,9);
    }
    // std::cout << testPtNorm << std::endl;

    // std::cout << computeF(testPtNorm) << std::endl;
    Eigen::MatrixXd F = computeF(testPtNorm);
    
    // // Get a rotation and translation matrix for each camera with respect to the last camera
    // Eigen::Matrix3d rotMats [nCams-1];
    // Eigen::Vector3d transVecs [nCams-1];
    // for(int ii=1; ii<nCams; ii++){
    //     Eigen::Matrix3d rotTemp;
    //     Eigen::Vector3d transTemp;
    //     // Chop out the appropriate cameras to feed in
    //     Eigen::MatrixXd ptNormTemp(nPoints,4);
    //     ptNormTemp << ptNorm.col(0) << ptNorm.col(1) << ptNorm.col(2*ii) << ptNorm.col(2*ii+1);
    //     Eigen::MatrixXd camMatrix = computeF(ptNormTemp);
    //     double score = twoCamCal(ptNormTemp, camMatrix, rotTemp, transTemp);
    // }


}

// Getter methods go here

const std::vector<std::vector<double> > easyWand::getWandPoints() const {
    return wandPts;
}

const int easyWand::getNumCams() const{
    return numCameras;
}

// Utility functions go here
Eigen::MatrixXd VectToEigenMat(const std::vector<double> inputvector){
    int rows = inputvector.size();
    Eigen::MatrixXd outputvec(rows,1);
    for(int ii=0; ii < rows; ii++){
        outputvec(ii,0) = inputvector[ii];
    } 
    return outputvec;
}

Eigen::MatrixXd VectToEigenMat(const std::vector< std::vector <double> > inputvector) {
    int rows = inputvector.size();
    if(rows<1){
        std::cerr << "Tried to convert an empty vector to Eigen matrix" << std::endl;
    }
    int cols = inputvector[0].size();
    Eigen::MatrixXd outputmat(rows, cols);
    for(int ii=0; ii < rows; ii++){
        for(int jj=0; jj < cols; jj++){
            outputmat(ii,jj) = inputvector[ii][jj];
        }
    }
    return outputmat;
}

Eigen::MatrixXd computeF(const Eigen::MatrixXd &ptNormAccess){
    
    Eigen::MatrixXd ptNorm = Eigen::MatrixXd(ptNormAccess);
    // std::cout << ptNorm << std::endl;
    // Set the average position to the center, keep track of scale!
    Eigen::VectorXd avgvals(4);
    for(int ii=0; ii < ptNorm.cols(); ii++){
        double avgval = ptNorm.col(ii).mean();
        avgvals(ii) = avgval;
        for(int jj=0; jj < ptNorm.rows(); jj++){
            ptNorm(jj,ii) -= avgval;
        }
    }

    // TODO: Scale the points so that the average distance from the center is sqrt(2)
    double scale1 = 1.0;
    double scale2 = 1.0;

    // Construct matrix A
    Eigen::MatrixXd A(ptNorm.rows(),9);
    for(int ii=0; ii < ptNorm.rows(); ii++){
        A(ii,0) = ptNorm(ii,2)*ptNorm(ii,0);
        A(ii,1) = ptNorm(ii,2)*ptNorm(ii,1);
        A(ii,2) = ptNorm(ii,2);
        A(ii,3) = ptNorm(ii,3)*ptNorm(ii,0);
        A(ii,4) = ptNorm(ii,3)*ptNorm(ii,1);
        A(ii,5) = ptNorm(ii,3);
        A(ii,6) = ptNorm(ii,0);
        A(ii,7) = ptNorm(ii,1);
        A(ii,8) = 1.0;
    }
    // Do the first decomposition for initial (non-orthogonal) estimate
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeFullV);
    // Pull the estimate from the last column of V
    Eigen::MatrixXd pulledcolumn = svd.matrixV().col(8);
    pulledcolumn.resize(3,3);

    // Next step is given as Let F = UDVt be the SVD of F, where D is a diagonal matrix D=diag(r,s,t) satisfying r> s> t (the singular values of F).  Then F'=U*diag(r,s,0)*Vt minimizes the Frobenius norm of F-F' [8 point algorithm]
    Eigen::JacobiSVD<Eigen::MatrixXd> svd2(pulledcolumn, Eigen::ComputeFullU | Eigen::ComputeFullV);
    // Eigen::Matrix3d D = svd2.singularValues();
    Eigen::Matrix3d newD;
    newD(0,0) = svd2.singularValues()(0);
    newD(1,1) = svd2.singularValues()(1);
    Eigen::Matrix3d U = svd2.matrixU();
    Eigen::Matrix3d V = svd2.matrixV();
    V.transposeInPlace();
    Eigen::MatrixXd F = U * newD * V;

    // Return the original scale 
    Eigen::Matrix3d S1;
    S1 << scale1, 0, -scale1*avgvals(0),
    0, scale1, -scale1*avgvals(1), 
    0, 0, 1;
    Eigen::Matrix3d S2;
    S2 << scale2, 0, -scale2*avgvals(2),
    0, scale2, -scale2*avgvals(3),
    0, 0, 1;
    S1.transposeInPlace();
    F =  S1*F*S2;
    for(int ii=0; ii<3; ii++){
        for(int jj=0; jj<3; jj++){
            F(ii,jj) /= F(2,2);
        }
    }
    return F;
}

double twoCamCal(const Eigen::MatrixXd ptNorm, Eigen::Matrix3d camMatrix, Eigen::Matrix3d rMat, Eigen::Vector3d tVec) {
    // Assuming camera matrix already obtained from computeF

    return 0;

}


int main(int argc, char* argv[]){
    // Check the number of inputs
    if (argc < 3) {
        // Tell the user how to run the program
        std::cerr << "Usage: " << argv[0] << " CAMERA_PROFILE.txt WAND_PTS_FILE.csv BKGD_PTS_FILE" << std::endl;
        return 1;
    }

    // Set up the easyWand calibration object from input files
    easyWand wanda = easyWand(argv[1], argv[2], argv[3]);

    // Try and view the wand points
    wanda.plotWandPoints(wanda.getWandPoints(), 1);

    // Try and do an actual calibration routine.... wheeeeeeeee
    wanda.computeCalibration();

    // Array testing :\

    return 0;
}
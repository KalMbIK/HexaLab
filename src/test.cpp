#include <app.h>
#include <plane_filter.h>
#include <quality_filter.h>
#include <peeling_filter.h>
#include <pick_filter.h>
#include "json.hpp"
#include <fstream>

using namespace std;
using namespace HexaLab;
using json = nlohmann::json; // for json reading

int main() {
    printf ( "Size of Vector3 %lu\n", sizeof ( Eigen::Vector3f ) );

    printf ( "Size of Cell %lu\n", sizeof ( Cell ) );
    printf ( "Size of Face %lu\n", sizeof ( Face ) );
    //printf ( "Size of Edge %lu\n", sizeof ( Edge ) );
    printf ( "Size of Vert %lu\n", sizeof ( Vert ) );
    //printf ( "Size of Dart %lu\n", sizeof ( Dart ) );
    
    // Testing all the meshes in the repository:
    // The repository is indexed by a JSON that contains an array named 'sources' that contains papers. 
    // each paper has a 'data' that is an array of mesh names; 
    
    std::ifstream istr ( "../datasets/index.json" );
    if ( !istr.is_open() ) {
        printf ( "Failing to opening dataset index" );
        exit ( -1 );
    }

    json repositoryJSON;
    istr >> repositoryJSON;
    json paperArrayJSON = repositoryJSON["sources"];
    int meshCnt = 0;
    size_t hexMin= UINT_MAX;
    size_t hexMax = 0;
    std::string minMeshName, maxMeshName;
    int failCnt = 0;
    App app;
    for ( size_t i = 0; i < paperArrayJSON.size(); i++ ) {
//    { size_t i= 9;
        printf ( "Dataset %lu %lu \n", i, paperArrayJSON[i]["paper"].size() );
        json paper = paperArrayJSON[i]["paper"];
        string path = paperArrayJSON[i]["path"];
        string title = paper["title"];
        json dataVec = paperArrayJSON[i]["data"];
        printf ( "-- title %s\n", title.c_str() );

      for ( size_t j = 0; j < dataVec.size(); ++j ) {
//        { size_t j= 2;
            printf ("Mesh %lu/%lu on dataset %lu/%lu\n",j+1,dataVec.size(),i+1,paperArrayJSON.size());fflush ( stdout );
            ++meshCnt;
            string filename = dataVec[j];//"jumpRamp.mesh";
            const string basepath = "../datasets/";
            app.set_quality_measure( QualityMeasureEnum::SJ );
            //bool ret = app.import_mesh ( basepath + "/example_tetra.mesh" );
            bool ret = app.import_mesh ( basepath + path + "/" + filename );

            if(!ret) {
              failCnt++;
              meshCnt--;
              printf("\n\n ************** FAILURE ***************\n in loading mesh  %s\n\n",filename.c_str());
            }
            
            size_t hexNum = app.get_mesh()->cells.size();
            if(hexNum > hexMax) {
              hexMax=hexNum;
              maxMeshName=path + "/" + filename;
            }
            if(hexNum < hexMin) {
              hexMin=hexNum;
              minMeshName=path + "/" + filename;
            }
           
            printf("     Hexas: %4lu\n",app.get_mesh()->cells.size());
            printf("     Faces: %4lu\n",app.get_mesh()->faces.size());
            printf("TOTAL RAM: %4lu\n\n",app.get_mesh()->total_occupation_RAM() );

            fflush ( stdout );

            //break;
        }
        //break;
    }
    
    printf ( "%i meshes in a collection of %i papers (%i fails to load)\n", meshCnt, int(paperArrayJSON.size()), failCnt );
    printf ( "Range of Meshes\n"
             "   Min %10lu hex (%s) \n"
             "   Max %10lu hex (%s) \n", hexMin, minMeshName.c_str(),hexMax,maxMeshName.c_str());
    printf ( "Press enter to exit.\n" );
}

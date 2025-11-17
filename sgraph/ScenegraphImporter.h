#ifndef _SCENEGRAPHIMPORTER_H_
#define _SCENEGRAPHIMPORTER_H_

#include "IScenegraph.h"
#include "Scenegraph.h"
#include "GroupNode.h"
#include "LeafNode.h"
#include "TransformNode.h"
#include "RotateTransform.h"
#include "ScaleTransform.h"
#include "TranslateTransform.h"
#include "KeyframeTransform.h"
#include "PolygonMesh.h"
#include "Material.h"
#include <Light.h>
#include <istream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <iostream>
#include <stdexcept>
using namespace std;

namespace sgraph {
    class ScenegraphImporter {
        public:
            ScenegraphImporter() {
                root = NULL;
            }

            IScenegraph *parse(istream& input) {
                string command;
                string inputWithOutCommentsString = stripComments(input);
                istringstream inputWithOutComments(inputWithOutCommentsString);
                while (inputWithOutComments >> command) {
                    cout << "Read " << command << endl;
                    if (command == "instance") {
                        string name,path;
                        inputWithOutComments >> name >> path;
                        cout << "Read " << name << " " << path << endl;
                        meshPaths[name] = path;
                        ifstream in(path);
                       if (in.is_open()) {
                        util::PolygonMesh<VertexAttrib> mesh = util::ObjImporter<VertexAttrib>::importFile(in,false);
                        meshes[name] = mesh;         
                       } 
                    }
                    else if (command == "group") {
                        parseGroup(inputWithOutComments);
                    }
                    else if (command == "leaf") {
                        parseLeaf(inputWithOutComments);

                    }
                    else if (command == "material") {
                        parseMaterial(inputWithOutComments);
                    }
                    else if (command == "scale") {
                        parseScale(inputWithOutComments);
                    }
                    else if (command == "rotate") {
                        parseRotate(inputWithOutComments);
                    }
                    else if (command == "translate") {
                        parseTranslate(inputWithOutComments);
                    }
                    else if (command == "keyframes") {
                        parseKeyframes(inputWithOutComments);
                    }
                    else if (command == "copy") {
                        parseCopy(inputWithOutComments);
                    }
                    else if (command == "import") {
                        parseImport(inputWithOutComments);
                    }
                    else if (command == "assign-material") {
                        parseAssignMaterial(inputWithOutComments);
                    }
                    else if (command == "light") {
                        parseLight(inputWithOutComments);
                    }
                    else if (command == "assign-light") {
                        parseAssignLight(inputWithOutComments);
                    }
                    else if (command == "add-child") {
                        parseAddChild(inputWithOutComments);
                    }
                    else if (command == "assign-root") {
                        parseSetRoot(inputWithOutComments);
                    }
                    else {
                        throw runtime_error("Unrecognized or out-of-place command: "+command);
                    }
                }
                if (root!=NULL) {
                    IScenegraph *scenegraph = new Scenegraph();
                    scenegraph->makeScenegraph(root);
                    scenegraph->setMeshes(meshes);
                    scenegraph->setMeshPaths(meshPaths);
                    return scenegraph;
                }
                else {
                    throw runtime_error("Parsed scene graph, but nothing set as root");
                }
            }
            protected:
                virtual void parseGroup(istream& input) {
                    string varname,name;
                    input >> varname >> name;
                    
                    cout << "Read " << varname << " " << name << endl;
                    SGNode *group = new GroupNode(name,NULL);
                    nodes[varname] = group;
                }

                virtual void parseLeaf(istream& input) {
                    string varname,name,command,instanceof;
                    input >> varname >> name;
                    cout << "Read " << varname << " " << name << endl;
                    input >> command;
                    if (command == "instanceof") {
                        input >> instanceof;
                    }
                    SGNode *leaf = new LeafNode(instanceof,name,NULL);
                    nodes[varname] = leaf;
                } 

                virtual void parseScale(istream& input) {
                    string varname,name;
                    input >> varname >> name;
                    float sx,sy,sz;
                    input >> sx >> sy >> sz;
                    SGNode *scaleNode = new ScaleTransform(sx,sy,sz,name,NULL);
                    nodes[varname] = scaleNode;
                }

                virtual void parseTranslate(istream& input) {
                    string varname,name;
                    input >> varname >> name;
                    float tx,ty,tz;
                    input >> tx >> ty >> tz;
                    SGNode *translateNode = new TranslateTransform(tx,ty,tz,name,NULL);
                    nodes[varname] = translateNode;         
                }

                virtual void parseRotate(istream& input) {
                    string varname,name;
                    input >> varname >> name;
                    float angleInDegrees,ax,ay,az;
                    input >> angleInDegrees >> ax >> ay >> az;
                    SGNode *rotateNode = new RotateTransform(glm::radians(angleInDegrees),ax,ay,az,name,NULL);
                    nodes[varname] = rotateNode;         
                }

                virtual void parseKeyframes(istream& input) {
                    string varname,name;
                    input >> varname >> name;
                    string filepath;
                    input >> filepath;

                    ifstream keyframepathfile(filepath);
                    if (keyframepathfile.is_open()) {
                        SGNode *keyframeNode = readKeyframes(varname,keyframepathfile);
                        nodes[varname] = keyframeNode; 
                    }
                }

                virtual void parseMaterial(istream& input) {
                    util::Material mat;
                    float r,g,b;
                    string name;
                    input >> name;
                    string command;
                    input >> command;
                    while (command!="end-material") {
                        if (command == "ambient") {
                            input >> r >> g >> b;
                            mat.setAmbient(r,g,b);
                        }
                        else if (command == "diffuse") {
                            input >> r >> g >> b;
                            mat.setDiffuse(r,g,b);
                        }
                        else if (command == "specular") {
                            input >> r >> g >> b;
                            mat.setSpecular(r,g,b);
                        }
                        else if (command == "emission") {
                            input >> r >> g >> b;
                            mat.setEmission(r,g,b);
                        }
                        else if (command == "shininess") {
                            input >> r;
                            mat.setShininess(r);
                        }
                        input >> command;
                    }
                    materials[name] = mat;
                }

                virtual void parseCopy(istream& input) {
                    string nodename,copyof;

                    input >> nodename >> copyof;
                    if (nodes.find(copyof)!=nodes.end()) {
                        SGNode * copy = nodes[copyof]->clone();
                        nodes[nodename] = copy;
                    }
                }

                virtual void parseImport(istream& input) {
                    string nodename,filepath;

                    input >> nodename >> filepath;
                    ifstream external_scenegraph_file(filepath);
                    if (external_scenegraph_file.is_open()) {
                        
                        IScenegraph *importedSG = parse(external_scenegraph_file);
                        nodes[nodename] = importedSG->getRoot();
                       /* for (map<string,util::PolygonMesh<VertexAttrib> >::iterator it=importedSG->getMeshes().begin();it!=importedSG->getMeshes().end();it++) {
                            this->meshes[it->first] = it->second;
                        }
                        for (map<string,string>::iterator it=importedSG->getMeshPaths().begin();it!=importedSG->getMeshPaths().end();it++) {
                            this->meshPaths[it->first] = it->second;
                        }
                        */
                        //delete the imported scene graph but not its nodes!
                        importedSG->makeScenegraph(NULL);
                        delete importedSG;
                    }
                }

                virtual void parseAssignMaterial(istream& input) {
                    string nodename,matname;
                    input >> nodename >> matname;

                    LeafNode *leafNode = dynamic_cast<LeafNode *>(nodes[nodename]);
                    if ((leafNode!=NULL) && (materials.find(matname)!=materials.end())) {
                        leafNode->setMaterial(materials[matname]);
                    }
                }

                virtual void parseLight(istream& input) {
                    string name;
                    input >> name;
                    util::Light light;
                    string property;
                    input >> property;
                    while (property!="end-light") {
                        if (property == "ambient") {
                            float r,g,b;
                            input >> r >> g >> b;
                            light.setAmbient(r,g,b);
                        }
                        else if (property == "diffuse") {
                            float r,g,b;
                            input >> r >> g >> b;
                            light.setDiffuse(r,g,b);
                        }
                        else if (property == "specular") {
                            float r,g,b;
                            input >> r >> g >> b;
                            light.setSpecular(r,g,b);
                        }
                        else if (property == "position") {
                            float x,y,z;
                            input >> x >> y >> z;
                            light.setPosition(x,y,z);
                        }
                        else if (property == "direction") {
                            float x,y,z;
                            input >> x >> y >> z;
                            light.setDirection(x,y,z);
                        }
                        else if (property == "spot-direction") {
                            float x,y,z;
                            input >> x >> y >> z;
                            light.setSpotDirection(x,y,z);
                        }
                        else if (property == "spot-angle") {
                            float angle;
                            input >> angle;
                            light.setSpotAngle(angle);
                        }
                        else {
                            throw runtime_error("Unrecognized light property: "+property);
                        }
                        input >> property;
                    }
                    lights[name] = light;
                }

                virtual void parseAssignLight(istream& input) {
                    string lightname,nodename;
                    input >> lightname >> nodename;
                    if ((nodes.find(nodename)!=nodes.end()) && (lights.find(lightname)!=lights.end())) {
                        nodes[nodename]->addLight(lights[lightname]);
                    }
                }

                virtual void parseAddChild(istream& input) {
                    string childname,parentname;

                    input >> childname >> parentname;
                    ParentSGNode * parentNode = dynamic_cast<ParentSGNode *>(nodes[parentname]);
                    SGNode * childNode = NULL;
                    if (nodes.find(childname)!=nodes.end()) {
                        childNode = nodes[childname];
                    }

                    if ((parentNode!=NULL) && (childNode!=NULL)) {
                        parentNode->addChild(childNode);
                    }
                }

                virtual void parseSetRoot(istream& input) {
                    string rootname;
                    input >> rootname;

                    root = nodes[rootname];

                    cout << "Root's name is "<< root->getName() << endl;
                }

                string stripComments(istream& input) {
                    string line;
                    stringstream clean;
                    while (getline(input,line)) {
                        int i=0;
                        while ((i<line.length()) && (line[i]!='#')) {
                            clean << line[i];
                            i++;
                        }
                        clean << endl;
                    }
                    return clean.str();
                }

                virtual SGNode *readKeyframes(string& name,ifstream& filename) {
                    SGNode *node = NULL;
                    vector<glm::vec4> positions;
                    vector<glm::vec4> up_vectors;

                    int numKeyframes;
                    filename >> numKeyframes;
                    for (int i=0;i<numKeyframes;i+=1) {
                        glm::vec4 position,up;

                        filename >> position.x >> position.y >> position.z;
                        position.w = 1;

                        filename >> up.x >> up.y >> up.z;
                        up.w = 0;
                        positions.push_back(position);
                        up_vectors.push_back(up);
                    }
                    return new KeyframeTransformNode(name,positions,up_vectors,NULL);
                }
            private:
                map<string,SGNode *> nodes;
                map<string,util::Material> materials;
                map<string,util::Light> lights;
                map<string,util::PolygonMesh<VertexAttrib> > meshes;
                map<string,string> meshPaths;
                SGNode *root;

        
    };
}


#endif

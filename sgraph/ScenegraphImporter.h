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
#include "PolygonMesh.h"
#include "Material.h"
#include <istream>
#include <map>
#include <string>
#include <iostream>
#include "../PPMImageLoader.h"
using namespace std;

namespace sgraph {
    class ScenegraphImporter {
        public:
            ScenegraphImporter() {

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
                    else if (command == "image") {
                        string name,path;
                        inputWithOutComments >> name >> path;
                        cout << "Read " << name << " " << path << endl;
                        ifstream in(path);
                        if (in.is_open()) {
                            ImageLoader *loader = new PPMImageLoader();
                            loader->load(path);
                            util::TextureImage * textureImage = new util::TextureImage(loader->getPixels(), loader->getWidth(), loader->getHeight(), name);
                            textures[name] = textureImage;         
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
                    else if (command == "light") {
                        parseLight(inputWithOutComments);
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
                    else if (command == "copy") {
                        parseCopy(inputWithOutComments);
                    }
                    else if (command == "import") {
                        parseImport(inputWithOutComments);
                    }                    
                    else if (command == "assign-texture") {
                        parseAssignTexture(inputWithOutComments);
                    }
                    else if (command == "assign-light") {
                        parseAssignLight(inputWithOutComments);
                    }
                    else if (command == "assign-material") {
                        parseAssignMaterial(inputWithOutComments);
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
                    scenegraph->setTextures(textures);
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
                        else if (command == "absorption") {
                            input >> r;
                            mat.setAbsorption(r);
                        }
                        else if (command == "reflection") {
                            input >> r;
                            mat.setReflection(r);
                        }
                        else if (command == "transparency") {
                            input >> r;
                            mat.setTransparency(r);
                        }
                        else if (command == "refractive") {
                            input >> r;
                            mat.setRefractiveIndex(r);
                        }
                        input >> command;
                    }
                    materials[name] = mat;
                }

                virtual void parseLight(istream& input) {
                    util::Light light;
                    light.setSpotAngle(180);
                    float r,g,b,a;
                    string name;
                    input >> name;
                    string command;
                    input >> command;
                    while (command!="end-light") {
                        if (command == "ambient") {
                            input >> r >> g >> b;
                            light.setAmbient(r,g,b);
                        }
                        else if (command == "diffuse") {
                            input >> r >> g >> b;
                            light.setDiffuse(r,g,b);
                        }
                        else if (command == "specular") {
                            input >> r >> g >> b;
                            light.setSpecular(r,g,b);
                        }
                        else if (command == "position") {
                            input >> r >> g >> b;
                            light.setPosition(r,g,b);
                        }
                        else if (command == "direction") {
                            input >> r >> g >> b;
                            light.setDirection(r,g,b);
                        }
                        else if (command == "spot-direction") {
                             input >> r >> g >> b;
                            light.setSpotDirection(r,g,b);
                        }
                        else if (command == "spot-angle") {
                             input >> r;
                            light.setSpotAngle(r);
                        }
                        input >> command;
                        cout << "Now read " << command;
                    }
                    lights[name] = light;
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

                virtual void parseAssignTexture(istream& input) {
                    string nodename,textureName;
                    input >> nodename >> textureName;

                    LeafNode *leafNode = dynamic_cast<LeafNode *>(nodes[nodename]);
                    if ((leafNode!=NULL) && (textures.find(textureName)!=textures.end())) {
                        leafNode->setTextureName(textureName);
                    }
                }


                virtual void parseAssignLight(istream& input) {
                    string nodename,lightname;
                    input >> lightname >> nodename;

                    LeafNode *leafNode = dynamic_cast<LeafNode *>(nodes[nodename]);
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
            private:
                map<string,SGNode *> nodes;
                map<string,util::Material> materials;
                map<string,util::Light> lights;
                map<string,util::PolygonMesh<VertexAttrib> > meshes;
                map<string,string> meshPaths;
                map<string,util::TextureImage*> textures;
                SGNode *root;

        
    };
}


#endif
#ifndef _PARENTSGNODE_H_
#define _PARENTSGNODE_H_

#include "AbstractSGNode.h"

namespace sgraph {
    /**
     * This class represents an SGNode that can have children
     * 
     */
    class ParentSGNode: public AbstractSGNode {
        public:
        ParentSGNode(const string& name,IScenegraph *scenegraph)
        :AbstractSGNode(name,scenegraph) {}

        ~ParentSGNode() {
            for (int i=0;i<children.size();i++) {
                delete children[i];
            }
        }
        virtual void addChild(SGNode *child)=0;
        vector<SGNode *> getChildren() {
            return children;
        }

        /**
         * Searches recursively into its subtree to look for node with specified name.
         * \param name name of node to be searched
         * \return the node whose name this is if it exists within this subtree, null otherwise
         */
        SGNode *getNode(const string& name) {
            SGNode *n = AbstractSGNode::getNode(name);
            if (n!=NULL) {
                return n;
            }

            int i=0;
            SGNode *answer = NULL;

            while ((i<children.size()) && (answer == NULL)) {
                answer = children[i]->getNode(name);
                i++;
            }
            return answer;
        }

        /**
             * Creates a deep copy of the subtree rooted at this node
             * \return a deep copy of the subtree rooted at this node
             */
            SGNode *clone() {

                ParentSGNode * newtransform = copyNode();

                for (int i=0;i<children.size();i=i+1) {
                    newtransform->addChild(children[i]->clone());
                }

                return newtransform;
            }
        protected:
        vector<SGNode *> children;

        virtual ParentSGNode *copyNode()=0;
    };
}
#endif
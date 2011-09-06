#ifndef CONVERSION_TO_ACTIONS_H
#define CONVERSION_TO_ACTIONS_H


#include "CBlobTreeNetwork.h"
#include "CLayerManager.h"

//namespace PS{
	//class CBlobTreeToActions;
//}
using namespace PS;
//Converts a BlobTree to a binary tree ready and
//then to a list of actions which can be saved as a file
class CBlobTreeToActions
{
private:
	QStringList m_lstActions;
	CLayer* m_Layer;

	//int recursive_countErrors(CBlobTree* node);
	//int recursive_convertToBinaryTree(CBlobTree* node, CBlobTree* clonned);
	int recursive_nodeToAction(CBlobTree* node);

	bool appendCommand( SKETCHCMD command, CBlobTree* node, vec4f param);
public:
	CBlobTreeToActions(CLayer* input);
	~CBlobTreeToActions();

	bool convert(const char* chrOutFileName = NULL);
};



#endif
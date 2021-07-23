
// LeftView.cpp : implementation of the CLeftView class
//

#include "stdafx.h"
#include "winstyle.h"

#include "winstyleDoc.h"
#include "LeftView.h"
#include"Command_Extractor.h"
#include"CommandMapper.h"
#include<locale>
#include"MainFrm.h"
#include<queue>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
END_MESSAGE_MAP()


// CLeftView construction/destruction

CLeftView::CLeftView()
{
	// TODO: add construction code here
}

CLeftView::~CLeftView()
{
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs

	return CTreeView::PreCreateWindow(cs);
}
std::string Convert(std::string& str)
{
	std::locale settings;
	std::string converted;

	for (short i = 0; i < str.size(); ++i)
		converted += (std::toupper(str[i], settings));

	return converted;
}
void CLeftView::OnInitialUpdate()
{

	struct abc
	{
		size_t offset;

	};
	CTreeView::OnInitialUpdate();
	
	StreamReader st;
	ifstream in;
	MappingCommand mc;
	mc.fillRefCmd();
	in.open("TempPacPayload.txt");
	if (in.good()) {
		in.close();
		StreamReader *pStreamReader = new StreamReader();
		std::string str("TempPacPayload.txt");
		pStreamReader->init(str);
		std::vector<ipds_command *> vectCmd;
		while (pStreamReader->GetNextField(vectCmd))
		{

		}

		CTreeCtrl &pTreeCtrl = GetTreeCtrl();
		pTreeCtrl.ModifyStyle(NULL, TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS);
		HTREEITEM hRoot = pTreeCtrl.GetRootItem();
		HTREEITEM controll = pTreeCtrl.InsertItem("Controll Command", hRoot);
		bool pageFlag=false;
		for (int i = 0; i < pStreamReader->cmdVect.size()-1; i++)
		{
			std::stringstream stream;
			stream << std::hex << pStreamReader->cmdVect[i]->command;
			std::string result(stream.str());
			result = Convert(result);
			result = "X'" + result + "'";
			ipds_cmd_ref icr = mc.getCmdAssoc(result);

			HTREEITEM ht2 = pTreeCtrl.InsertItem(icr.cmd.c_str(), controll);
			
			if (pStreamReader->cmdVect[i]->command == 54959)
			{
				std::stringstream stream;
				stream << std::hex << pStreamReader->cmdVect[i]->command;
				std::string result(stream.str());
				if (result.length() < 4)
				{
					continue;
				}
				result = Convert(result);
				result = "X'" + result + "'";
				ipds_cmd_ref icr=mc.getCmdAssoc(result);
			
				HTREEITEM ht = pTreeCtrl.InsertItem(icr.cmd.c_str(), hRoot);

				while (i < pStreamReader->cmdVect.size()-1) {
					std::stringstream stream;
				stream << std::hex << pStreamReader->cmdVect[i]->command;
				std::string result(stream.str());
				result = Convert(result);
				result = "X'" + result + "'";
				ipds_cmd_ref icr = mc.getCmdAssoc(result);
				     
					HTREEITEM ht2 = pTreeCtrl.InsertItem(icr.cmd.c_str(), ht);
					
					if((pStreamReader->cmdVect[i]->command == 54975)|| (pStreamReader->cmdVect[i+1]->command == 54959))//||(pStreamReader->cmdVect.size()==i-2))
					{
						break;
					}
					i++;
				}

			}
		}
		MessageBox("FILE PARSED", 0);
		CMainFrame* pmf = (CMainFrame*)AfxGetMainWnd();
		
	}else{}
	

	

	// TODO: You may populate your TreeView with items by directly accessing
	//  its tree control through a call to GetTreeCtrl().
}


// CLeftView diagnostics

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CwinstyleDoc* CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CwinstyleDoc)));
	return (CwinstyleDoc*)m_pDocument;
}
#endif //_DEBUG


// CLeftView message handlers

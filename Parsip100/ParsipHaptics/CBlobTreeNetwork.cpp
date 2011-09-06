#include "CBlobTreeNetwork.h"
#include "PS_FrameWork/include/PS_ErrorManager.h"
#include "PS_FrameWork/include/PS_String.h"
#include "PS_FrameWork/include/_dataTypes.h"
#include "PS_BlobTree/include/CSkeletonPrimitive.h"
#include "_GlobalFunctions.h"

namespace PS
{
	CDesignNet* CDesignNet::sm_pDesignNet = NULL;
	CSketchNetCommandTranslator* CSketchNetCommandTranslator::sm_pCmdTranslator = NULL;

	CMember::CMember( const QString& strIPAddress, quint16 port /*= DEFAULT_PORT_NUM*/ )
	{
		m_strAddress = strIPAddress;
		m_strAlias = strIPAddress;
		m_port = port;
		m_ctRecv = 0;
		m_ctSent = 0;
		m_ctErrors = 0;
		m_bytesSent = 0;
		m_bAlive = false;
		m_bLogSend = true;
		connect(&m_socket, SIGNAL(connected()), this, SLOT(actNetConnected()));	
		connect(&m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(actNetBytesWritten(qint64)));
		connect(&m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(actNetDisplayError(QAbstractSocket::SocketError)));	
		m_socket.connectToHost(m_strAddress, m_port);
	}


	void CMember::connectToHost()
	{
		m_socket.connectToHost(m_strAddress, m_port);
	}

	bool CMember::sendText( QString strText )
	{
		if(m_bAlive && m_socket.isValid())
		{
			strText += "\r\n";
			quint64 nWritten = m_socket.write(strText.toUtf8());
			if(nWritten > 0)
			{
				if(m_bLogSend)					
				{
					m_lstLogSend.append(strText);
					if(m_lstLogSend.size() > MAX_LOG_SIZE)						
					{
						saveLog();
						m_lstLogSend.clear();
					}
				}
				activate();
				m_ctSent++;				
				return true;
			}
			else
			{
				m_bAlive = false;
				emit downgrade();
			}
		}
		return false;
	}

	bool CMember::sendPacket( SKETCHCMDPACKET& packet )
	{
		QString strMsg;

		//Set MessageID in packet
		packet.msgID = m_ctSent;			
		//Convert Packet to string
		CSketchNetCommandTranslator::GetCmdTranslator()->translatePacketToStr(packet, strMsg);

		//Store a copy of packet in transmitted buffer and send text version
		m_txPackets.push_back(packet);
		return sendText(strMsg);
	}

	int CMember::recvAck( const SKETCHCMDPACKET& packet )
	{
		if(packet.cmd != cmdAck) return SKETCHNET_ACK_NOT_VALID;
		SKETCHCMDPACKET p;
		for(size_t i=0; i<m_txPackets.size(); i++)
		{
			p = m_txPackets[i];
			if(p.msgID == packet.msgID)
			{				
				m_txPackets.remove(i);			
				return packet.ack;
			}
		}
		
		return SKETCHNET_ACK_NOT_VALID;
	}


	void CMember::actNetConnected()
	{
		m_bAlive = true;

		//Resolve URL to IP
		m_strAddress = m_socket.peerAddress().toString();
		emit upgrade();
	}

	void CMember::actNetDisplayError(QAbstractSocket::SocketError socketError)
	{
		m_bAlive = false;
		m_ctErrors++;

		
		if (socketError != QTcpSocket::RemoteHostClosedError)
		{
			DAnsiStr strError = printToAStr("Network Error: %s", m_socket.errorString().toAscii().data());
			ReportError(strError.ptr());
			FlushAllErrors();
		}
		
		emit downgrade();
	}

	void CMember::actNetBytesWritten( quint64 written )
	{
		m_bytesSent += written;
	}

	void CMember::activate()
	{
		m_lastActTime = QTime::currentTime();
		
		if(m_bAlive == false)
		{
			m_bAlive = true;
			emit upgrade();
		}		
	}

	void CMember::disconnect()
	{
		m_socket.close();
	}

	void CMember::saveLog()
	{
		QString alias = m_strAddress.replace(".", "_");
		QFile fOut(QString("C:\\SketchNet_Member%1.txt").arg(alias));
		if (fOut.open(QFile::Append | QFile::WriteOnly | QFile::Text)) 
		{
			QTextStream s(&fOut);
			for (int i = 0; i < m_lstLogSend.size(); ++i)
				s << m_lstLogSend.at(i) << '\n';
			m_lstLogSend.clear();
		} 
		else 
		{			
			return;
		}
		fOut.close();
	}
	//////////////////////////////////////////////////////////////////////////	
	CMember* CDesignNet::findMember(const QString& strAlias, int* pIndex, int* pIsPending )
	{
		CMember* a = NULL;
		for(size_t i=0; i<m_lstMembers.size();i++)
		{
			a = m_lstMembers[i];
			if((a->m_strAlias == strAlias)||(a->m_strAddress == strAlias))
			{
				if(pIndex) *pIndex = i;
				if(pIsPending) *pIsPending = 0;
				return m_lstMembers[i];
			}
		}
		//Pending
		for(size_t i=0; i<m_lstPending.size();i++)
		{
			a = m_lstPending[i];
			if((a->m_strAlias == strAlias)||(a->m_strAddress == strAlias))
			{
				if(pIndex) *pIndex = i;
				if(pIsPending) *pIsPending = 1;
				return m_lstPending[i];
			}
		}
		return NULL;
	}

	bool CDesignNet::addMember(const QString& strAddress)
	{		
		if(strAddress.length() == 0) return false;		
		CMember* m = findMember(strAddress);		
		if(m != NULL) 
		if(m->m_ctErrors == 0)
		{
			m->activate();
			return false;
		}

		m = new CMember(strAddress, m_port);
		connect(m, SIGNAL(upgrade()), this, SLOT(actNetUpgradeToMembers()));
		connect(m, SIGNAL(downgrade()), this, SLOT(actNetDowngradeToPending()));
		m_lstPending.push_back(m);
		return true;
	}

	bool CDesignNet::start()
	{
		if(m_tcpServer.isListening()) return false;

		connect(&m_tcpServer, SIGNAL(newConnection()), this, SLOT(actNetAcceptConnection()));

		if(!m_tcpServer.isListening() && !m_tcpServer.listen(QHostAddress::Any, m_port))
		{
			ReportError("Unable to connect server!");
			return false;
		}
		return true;
	}

	void CDesignNet::stop()
	{
		m_tcpServer.close();
		removeAll();
	}

	bool CDesignNet::removeMember( const QString& strAlias )
	{
		int index;
		CMember* mem = findMember(strAlias, &index);
		if(mem != NULL)
		{ 			
			m_lstMembers.remove(index);
			SAFE_DELETE(mem);
			return true;
		}
		else
			return false;
	}

	bool CDesignNet::removeMember(CMember* a )
	{
		if(a== NULL) return false;
		for(size_t i=0; i<m_lstMembers.size(); i++)
		{
			if(m_lstMembers[i] == a)
			{
				m_lstMembers.erase(m_lstMembers.begin() + i);
				SAFE_DELETE(a);
				return true;
			}
		}
		
		return false;
	}

	bool CDesignNet::removeMemberByName( const QString& strAlias )
	{
		CMember* a = NULL;
		for(size_t i=0; i<m_lstMembers.size(); i++)
		{
			a = m_lstMembers[i];
			if(a->m_strAlias == strAlias)
			{
				m_lstMembers.erase(m_lstMembers.begin() + i);
				SAFE_DELETE(a);
				emit sig_memberslist(getNames());
				return true;
			}
		}

		return false;
	}

	void CDesignNet::removeAll()
	{		
		CMember *a = NULL;
		for(size_t i=0; i<m_lstMembers.size(); i++)
		{	
			a = m_lstMembers[i];
			SAFE_DELETE(a);
		}
		for(size_t i=0; i<m_lstPending.size(); i++)
		{	
			a = m_lstPending[i];
			SAFE_DELETE(a);
		}
		m_lstMembers.clear();
		m_lstPending.clear();
	}

	void CDesignNet::actNetAcceptConnection()
	{
		m_tcpServerConnection = m_tcpServer.nextPendingConnection();
		if(m_tcpServerConnection == NULL) return;
		connect(m_tcpServerConnection, SIGNAL(readyRead()), this, SLOT(actNetServerReadData()));
		connect(m_tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(actNetServerDisplayError(QAbstractSocket::SocketError)));
		//Add new participant if not in the list of participants
		QString strPeerAddress = m_tcpServerConnection->peerAddress().toString();
		addMember(strPeerAddress);
	}

	void CDesignNet::actNetServerReadData()
	{		
		if(m_tcpServerConnection == NULL) return;

		int ctBytesRecv = m_tcpServerConnection->bytesAvailable();
		QByteArray q;	q.resize(ctBytesRecv);
		q = m_tcpServerConnection->readAll();

		int index;
		CMember* sender = findMember(m_tcpServerConnection->peerAddress().toString(), &index);
		if(sender)
		{
			sender->activate();
			sender->incrRecv();

			QString strMsg(q);					

			QStringList lstMsgs = strMsg.split("\r\n", QString::SkipEmptyParts);
			for(int i=0; i<lstMsgs.size();i++)			
				emit sig_newMessage(index, lstMsgs[i]);
		}
	}

	void CDesignNet::actNetServerDisplayError( QAbstractSocket::SocketError socketError )
	{
		if(m_tcpServerConnection == NULL) return;
		DAnsiStr strError = printToAStr("Network Error: %s", m_tcpServerConnection->errorString().toAscii().data());
		ReportError(strError.ptr());
		FlushAllErrors();
	}

	CDesignNet* CDesignNet::GetDesignNet()
	{
		if(sm_pDesignNet == NULL)
		{
			sm_pDesignNet = new CDesignNet();
		}
		return sm_pDesignNet;
	}

	bool CDesignNet::sendMessageToMember( const QString& strDest, const QString& strMsg, bool bSendToAll /*= false*/ )
	{
		CMember* a = NULL;
		if(bSendToAll)
		{
			size_t ctDelivered = 0;
			for(size_t i=0; i<m_lstMembers.size();i++)
			{
				a = m_lstMembers[i];
				if(a->sendText(strMsg)) ctDelivered++;
			}
			return (ctDelivered == m_lstMembers.size());
		}
		else
		{
			a = findMember(strDest);
			if(a)	return a->sendText(strMsg);
		}

		return false;
	}

	bool CDesignNet::sendMessageToMember( const QString& strMsg, int idxMember )
	{
		CMember* a = NULL;
		if(idxMember == -1)
		{
			size_t ctDelivered = 0;
			for(size_t i=0; i<m_lstMembers.size();i++)
			{
				a = m_lstMembers[i];
				if(a->sendText(strMsg)) 
					ctDelivered++;
			}
			return (ctDelivered == m_lstMembers.size());
		}
		else
		{
			a = getMember(idxMember);
			if(a)	return a->sendText(strMsg);
		}

		return false;
	}


	bool CDesignNet::sendPacketToMember( SKETCHCMDPACKET& packet, int idxMember /*= -1*/ )
	{
		CMember* a = NULL;
		if(idxMember == -1)
		{
			size_t ctDelivered = 0;
			for(size_t i=0; i<m_lstMembers.size();i++)
			{
				a = m_lstMembers[i];
				if(a->sendPacket(packet)) 
					ctDelivered++;
			}
			return (ctDelivered == m_lstMembers.size());
		}
		else
		{
			a = getMember(idxMember);
			if(a)	return a->sendPacket(packet);
		}

		return false;
	}

	int CDesignNet::actNetUpgradeToMembers()
	{
		if(m_lstPending.size() == 0) return 0;
		CMember* a= NULL;
		
		int count = 0;
		size_t i=0;
		while(i<m_lstPending.size())
		{
			a = m_lstPending[i];
			if(a->isAlive()) 
			{
				m_lstPending.detach(m_lstPending.begin() + i);
				m_lstMembers.push_back(a);
				count++;
			}
			else			
				i++;
		}
		
		if(count > 0) emit sig_memberslist(getNames());
		return count;
	}

	int CDesignNet::actNetDowngradeToPending()
	{
		if(m_lstMembers.size() == 0) return 0;
		CMember* a= NULL;
		int count = 0;
		size_t i=0;
		while(i<m_lstMembers.size())
		{
			a = m_lstMembers[i];
			if(!a->isAlive()) 
			{
				m_lstMembers.detach(m_lstMembers.begin() + i);
				m_lstPending.push_back(a);				
				
				count++;
			}
			else			
				i++;
		}
		
		if(count > 0) emit sig_memberslist(getNames());
		return count;
	}

	QStringList CDesignNet::getNames() const
	{
		QStringList lstNames;
		lstNames.reserve(m_lstMembers.size());
		for (size_t i=0; i<m_lstMembers.size();i++)
			lstNames.append(m_lstMembers[i]->m_strAlias);
		return lstNames;
	}

	CMember* CDesignNet::getMember( int idxMember ) const
	{
		if((idxMember >= 0)&&(idxMember < (int)m_lstMembers.size()))
			return m_lstMembers[idxMember];
		else
			return NULL;
	}

	QStringList CDesignNet::getIPs() const
	{
		QStringList lstIPs;
		lstIPs.reserve(m_lstMembers.size());
		for (size_t i=0; i<m_lstMembers.size();i++)
			lstIPs.append(m_lstMembers[i]->m_strAddress);
		return lstIPs;
	}

	//////////////////////////////////////////////////////////////////////////
	CSketchNetCommandTranslator::CSketchNetCommandTranslator()
	{
		m_keywords.reserve(30);
		m_keywords.push_back(DAnsiStr("ADD"));
		m_keywords.push_back(DAnsiStr("OPERATOR"));
		m_keywords.push_back(DAnsiStr("DELETE"));
		m_keywords.push_back(DAnsiStr("LOCK"));
		m_keywords.push_back(DAnsiStr("UNLOCK"));
		m_keywords.push_back(DAnsiStr("MOVE"));
		m_keywords.push_back(DAnsiStr("SCALE"));
		m_keywords.push_back(DAnsiStr("ROTATE"));
		m_keywords.push_back(DAnsiStr("ACK"));	
		m_keywords.push_back(DAnsiStr("SET"));	
		m_DicOffsetOps = m_keywords.size();

		m_keywords.push_back(DAnsiStr("UNION"));
		m_keywords.push_back(DAnsiStr("BLEND"));
		m_keywords.push_back(DAnsiStr("DIFFERENCE"));
		m_keywords.push_back(DAnsiStr("INTERSECTION"));
		m_DicOffsetPrims = m_keywords.size();


		m_keywords.push_back(DAnsiStr("SPHERE"));
		m_keywords.push_back(DAnsiStr("CAPSULE"));
		m_keywords.push_back(DAnsiStr("WELCOME"));
		m_keywords.push_back(DAnsiStr("CUBE"));
		m_keywords.push_back(DAnsiStr("CYLINDER"));
		m_keywords.push_back(DAnsiStr("TORUS"));
		m_keywords.push_back(DAnsiStr("DISC"));
		m_keywords.push_back(DAnsiStr("CONE"));
		//////////////////////////////////////////////////////////////////////////
		//TX CMD Maps
		m_mapTXCommands[cmdAdd]		= "ADD";
		m_mapTXCommands[cmdOperator]= "OPERATOR";
		m_mapTXCommands[cmdDelete]  = "DELETE";
		m_mapTXCommands[cmdLock]    = "LOCK";
		m_mapTXCommands[cmdUnlock]  = "UNLOCK";
		m_mapTXCommands[cmdMove]    = "MOVE";
		m_mapTXCommands[cmdScale]	= "SCALE";
		m_mapTXCommands[cmdRotate]  = "ROTATE";		
		m_mapTXCommands[cmdAck]		= "ACK";
		m_mapTXCommands[cmdSet]		= "SET";
		
		//TX ACKS
		m_mapTXAcks[ackSuccess]      = "SUCCESS";
		m_mapTXAcks[ackIDNotFound]   = "ID_NOT_FOUND";
		m_mapTXAcks[ackIDNotLocked]  = "ID_NOT_LOCKED";
		m_mapTXAcks[ackIDLocked]     = "ID_LOCKED";

		//TX OP Maps
		m_mapTXOps[bntOpUnion]		= "UNION";
		m_mapTXOps[bntOpBlend]		= "BLEND";
		m_mapTXOps[bntOpRicciBlend]	= "BLEND";
		m_mapTXOps[bntOpDif]		= "DIFFERENCE";
		m_mapTXOps[bntOpSmoothDif]	= "DIFFERENCE";
		m_mapTXOps[bntOpIntersect]  = "INTERSECTION";

		//TX Prim Maps
		m_mapTXPrims[sktPoint]	  = "SPHERE";
		m_mapTXPrims[sktLine]	  = "CAPSULE";
		m_mapTXPrims[sktCube]	  = "CUBE";
		m_mapTXPrims[sktCylinder] = "CYLINDER";
		m_mapTXPrims[sktRing]	  = "TORUS";
		m_mapTXPrims[sktDisc]	  = "DISC";
		m_mapTXPrims[sktTriangle] = "CONE";

		//Errors
		//PS_ERROR err_noerror	 = "No Error.";
		//PS_ERROR err_blobnode_id = "BlobNode ID not found.";
		//PS_ERROR err_msg_id		 = "Message ID not found.";
		//PS_ERROR err_param		 = "Param not found.";
		//PS_ERROR err_left_child  = "Left child not found.";
		//PS_ERROR err_right_child = "right child not found.";
		//PS_ERROR err_unknown	 = "Unknown Command.";

		m_mapErrors[peNoError]    = "No Error.";
		m_mapErrors[peBlobNodeID] = "Node ID Not Found.";
		m_mapErrors[peMsgID]	  = "Msg ID Not Found.";
		m_mapErrors[peParam]	  = "Param Not Found.";
		m_mapErrors[peLChild]     = "Left Child Not Found.";
		m_mapErrors[peRChild]     = "Right Child Not Found.";
		m_mapErrors[peUnknown]    = "Unknown Command.";


		//RX Maps
		//Add all commands
		m_mapTokens["ADD"]		   = cmdAdd;
		m_mapTokens["OPERATOR"]    = cmdOperator;
		m_mapTokens["DELETE"]	   = cmdDelete;
		m_mapTokens["LOCK"]		   = cmdLock;
		m_mapTokens["UNLOCK"]	   = cmdUnlock;
		m_mapTokens["MOVE"]		   = cmdMove;
		m_mapTokens["SCALE"]	   = cmdScale;
		m_mapTokens["ROTATE"]	   = cmdRotate;
		m_mapTokens["ACK"]		   = cmdAck;
		m_mapTokens["SET"]		   = cmdSet;


		//Add all acks
		m_mapTokens["SUCCESS"]	     = ackSuccess;
		m_mapTokens["ID_NOT_FOUND"]  = ackIDNotFound;
		m_mapTokens["ID_NOT_LOCKED"] = ackIDNotLocked;		
		m_mapTokens["ID_LOCKED"]	 = ackIDLocked;				
		

		//Add all operators
		m_mapTokens["UNION"] = bntOpUnion;
		m_mapTokens["BLEND"] = bntOpBlend;
		m_mapTokens["DIFFERENCE"] = bntOpDif;
		m_mapTokens["INTERSECTION"] = bntOpIntersect;		

		//Add primitives
		m_mapTokens["SPHERE"]	= sktPoint;
		m_mapTokens["CAPSULE"]  = sktLine;
		m_mapTokens["CUBE"]		= sktCube;
		m_mapTokens["CYLINDER"] = sktCylinder;
		m_mapTokens["TORUS"]	= sktRing;
		m_mapTokens["DISC"]		= sktDisc;
		m_mapTokens["CONE"]		= sktTriangle;
	}

	bool CSketchNetCommandTranslator::translateStrToPacket( const QString& strInputCmd, SKETCHCMDPACKET& output )
	{
		DAnsiStr strCmd(strInputCmd.toAscii().data());
		DAnsiStr temp;
		size_t trans;
		vec4f param;

		int ctTokens = 0;
		
		//Convert to upper case
		strCmd.toUpper();		
		
		if(readString(strCmd, temp))
		{
			trans = m_mapTokens[temp.ptr()];						
			if(getTokenType(temp) == ttCommand)
			{
				output.cmd = (SKETCHCMD)trans;
				ctTokens++;
			}
		}

		int ctExpectedParams = 0;
		PROTOCOL_ERROR checkParams[8];

		if(ctTokens > 0)
		{
			switch(output.cmd)
			{
			case(cmdAdd):
				{					
					output.opType = bntPrimSkeleton;
					ctTokens += readString(strCmd, temp);
					output.primType = (SkeletonType)m_mapTokens[temp.ptr()];
					ctTokens += readInteger(strCmd, output.blobnodeID);
					ctTokens += readParam(strCmd, output.param);		
					ctTokens += readInteger(strCmd, output.msgID);	
					return (ctTokens == 5);
				}
				break;
			case(cmdOperator):
				{
					ctTokens += readString(strCmd, temp);
					output.opType = (BlobNodeType)m_mapTokens[temp.ptr()];					
					ctTokens += readInteger(strCmd, output.blobnodeID);
					ctTokens += readInteger(strCmd, output.leftChild);
					ctTokens += readInteger(strCmd, output.rightChild);
					ctTokens += readInteger(strCmd, output.msgID);		
					return (ctTokens == 6);
				}
				break;
			case(cmdDelete):
				{
					ctTokens += readInteger(strCmd, output.blobnodeID);
					ctTokens += readInteger(strCmd, output.msgID);	
					return (ctTokens == 3);
				}
				break;
			case(cmdLock):
				{
					ctTokens += readInteger(strCmd, output.blobnodeID);
					ctTokens += readInteger(strCmd, output.msgID);	
					return (ctTokens == 3);
				}
				break;
			case(cmdUnlock):
				{
					ctTokens += readInteger(strCmd, output.blobnodeID);
					ctTokens += readInteger(strCmd, output.msgID);	
					return (ctTokens == 3);
				}
				break;
			case(cmdMove):
				{
					ctTokens += readInteger(strCmd, output.blobnodeID);
					ctTokens += readParam(strCmd, output.param);
					ctTokens += readInteger(strCmd, output.msgID);	
					return (ctTokens == 4);
				}
				break;
			case(cmdScale):
				{
					ctTokens += readInteger(strCmd, output.blobnodeID);
					ctTokens += readParam(strCmd, output.param);
					ctTokens += readInteger(strCmd, output.msgID);	
					return (ctTokens == 4);
				}
				break;
			case(cmdRotate):
				{
					ctTokens += readInteger(strCmd, output.blobnodeID);
					ctTokens += readParam(strCmd, output.param);
					ctTokens += readInteger(strCmd, output.msgID);	
					return (ctTokens == 4);
				}
				break;
			case(cmdAck):
				{
					ctTokens += readString(strCmd, temp);
					output.ack = (SKETCHACK)m_mapTokens[temp.ptr()];
					ctTokens += readInteger(strCmd, output.msgID);	
					return (ctTokens == 3);
				}
				break;
			case(cmdSet):
				{
					ctTokens += readString(strCmd, temp);
					output.opType = (BlobNodeType)m_mapTokens[temp.ptr()];
					ctTokens += readInteger(strCmd, output.blobnodeID);
					ctTokens += readInteger(strCmd, output.msgID);	
					return (ctTokens == 4);
				}
				break;
			default:
				{					
					ctTokens += readInteger(strCmd, output.msgID);					
					return (ctTokens == 2);
				}
			}
		}
		return false;
	}

	//Translate an input Command Structure to a String
	bool CSketchNetCommandTranslator::translatePacketToStr( const SKETCHCMDPACKET& inputCmd, QString& strOutput )
	{
		strOutput = QString(m_mapTXCommands[inputCmd.cmd].c_str());
		switch(inputCmd.cmd)
		{
		case(cmdAdd):
			{
				QString strSkelet = QString(m_mapTXPrims[inputCmd.primType].c_str());
				strOutput += QString(" %1 %2 VEC3(%3, %4, %5) %6").arg(strSkelet).arg(inputCmd.blobnodeID).	\
				arg(inputCmd.param.x).arg(inputCmd.param.y).arg(inputCmd.param.z).arg(inputCmd.msgID);
			}
			break;
		case(cmdOperator):
			{
				QString strOp = QString(m_mapTXOps[inputCmd.opType].c_str());
				strOutput += QString(" %1 %2 %3 %4 %5").arg(strOp).arg(inputCmd.blobnodeID).	\
				arg(inputCmd.leftChild).arg(inputCmd.rightChild).arg(inputCmd.msgID);
			}
			break;
		case(cmdDelete):
			{
				strOutput += QString(" %1 %2").arg(inputCmd.blobnodeID).arg(inputCmd.msgID);
			}
			break;
		case(cmdLock):
			{
				strOutput += QString(" %1 %2").arg(inputCmd.blobnodeID).arg(inputCmd.msgID);
			}
			break;
		case(cmdUnlock):
			{
				strOutput += QString(" %1 %2").arg(inputCmd.blobnodeID).arg(inputCmd.msgID);
			}
			break;
		case(cmdMove):
			{
				strOutput += QString(" %1 VEC3(%2, %3, %4) %5").arg(inputCmd.blobnodeID).  
				arg(inputCmd.param.x).arg(inputCmd.param.y).arg(inputCmd.param.z).arg(inputCmd.msgID);
			}
			break;
		case(cmdScale):
			{
				strOutput += QString(" %1 VEC3(%2, %3, %4) %5").arg(inputCmd.blobnodeID).
					arg(inputCmd.param.x).arg(inputCmd.param.y).arg(inputCmd.param.z).arg(inputCmd.msgID);
			}
			break;
		case(cmdRotate):
			{
				strOutput += QString(" %1 QUAT(%2, %3, %4, %5) %6").arg(inputCmd.blobnodeID). 
					arg(inputCmd.param.x).arg(inputCmd.param.y).arg(inputCmd.param.z).arg(inputCmd.param.w).arg(inputCmd.msgID);
			}
			break;
		case(cmdAck):		
			{
				QString strAckMsg = QString(m_mapTXAcks[inputCmd.ack].c_str());
				strOutput = QString("ACK %1 %2").arg(strAckMsg).arg(inputCmd.msgID);
			}
			break;
		default:		
			{
				QString strAckMsg = QString(m_mapTXCommands[inputCmd.cmd].c_str());
				strOutput = QString("ACK %1 %2").arg(strAckMsg).arg(inputCmd.msgID);
			}
			break;
		}
		return true;
	}

	CSketchNetCommandTranslator::~CSketchNetCommandTranslator()
	{
		m_mapTXCommands.clear();
		m_mapTXOps.clear();
		m_mapTXPrims.clear();
		m_mapTXAcks.clear();
		m_mapTokens.clear();
		m_keywords.clear();
	}

	bool CSketchNetCommandTranslator::readInteger( DAnsiStr& strCmd, int& value )
	{
		size_t pos = -1;
		if(strCmd.lfind(' ', pos))
		{
			DAnsiStr temp = strCmd.substr(0, pos);
			strCmd = strCmd.substr(pos);
			strCmd.removeStartEndSpaces();
			value = atoi(temp.ptr());			
			return true;
		}
		else if(strCmd.length() > 0)
		{
			value = atoi(strCmd.ptr());
			strCmd = "";
			return true;
		}

		return false;
	}

	bool CSketchNetCommandTranslator::readParam( DAnsiStr& strCmd, vec4f& value )
	{
		float f[4];
		size_t pos;				
		int iComp = 0;
		DAnsiStr strTemp;		

		for(iComp=0;iComp<4;iComp++)
			f[iComp] = 0.0f;
		
		iComp = 0;
		if(strCmd.lfind('(', pos))
			strCmd = strCmd.substr(pos + 1);
		else
			return false;
		while(strCmd.lfind(',', pos))
		{
			strTemp = strCmd.substr(0, pos);
			strCmd = strCmd.substr(pos + 1);					
			strCmd.removeStartEndSpaces();
			f[iComp] = static_cast<float>(atof(strTemp.ptr()));
			iComp++;
		}				

		if(strCmd.length() >= 1 && iComp < 4)
		{
			if(strCmd.lfind(')', pos))
			{
				strTemp = strCmd.substr(0, pos);				
				strTemp.removeStartEndSpaces();
				strCmd = strCmd.substr(pos + 1);
				f[iComp] = static_cast<float>(atof(strTemp.ptr()));					
				iComp++;
			}	
			else
				return false;
		}

		strCmd.removeStartEndSpaces();
		value.set(f);
		return true;
	}

	bool CSketchNetCommandTranslator::readString( DAnsiStr& strCmd, DAnsiStr& value )
	{
		size_t pos = -1;
		if(strCmd.lfind(' ', pos))
		{
			value = strCmd.substr(0, pos);
			strCmd = strCmd.substr(pos);
			strCmd.removeStartEndSpaces();
			return true;
		}
		return false;

	}

	CSketchNetCommandTranslator* CSketchNetCommandTranslator::GetCmdTranslator()
	{
		if(sm_pCmdTranslator == NULL)
		{
			sm_pCmdTranslator = new CSketchNetCommandTranslator();
		}
		return sm_pCmdTranslator;
	}

	TOKENTYPE CSketchNetCommandTranslator::getTokenType( const DAnsiStr& token ) const
	{		
		int res = 0;
		for(res = 0; res < m_keywords.size(); res++)
		{
			if(token == m_keywords[res])
				break;
		}
		if(res < m_DicOffsetOps)
			return ttCommand;
		else if(res >= m_DicOffsetOps && res < m_DicOffsetPrims)
			return ttOperator;
		else if(res >= m_DicOffsetPrims && res < m_mapTokens.size())
			return ttPrimitive;
		else 			
			return ttNone;
	}



}
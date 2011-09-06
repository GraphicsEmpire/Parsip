#ifndef CBLOBTREE_NET_H
#define CBLOBTREE_NET_H

#include "DSystem/include/DContainers.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QStringList>
#include <QFile>
#include <QTime>
#include "PS_BlobTree/include/CBlobTree.h"
#include "PS_BlobTree/include/CSkeleton.h"
#include <map>

class QTime;
class QTcpServer;
class QTcpSocket;
class QStringList;

using namespace PS::BLOBTREE;

namespace PS{

typedef enum SKETCHCMD {cmdAdd, cmdOperator, cmdDelete, 
						cmdLock, cmdUnlock, 
						cmdMove, cmdScale, cmdRotate, 
						cmdAck,	cmdSet};

typedef enum SKETCHACK{ackSuccess = 1, ackIDNotFound = -1, ackIDNotLocked = -2, ackIDLocked = -3};

/*
#define SKETCHNET_ACK_SUCCESS		 1
#define SKETCHNET_ACK_ID_NOT_FOUND  -1
#define SKETCHNET_ACK_ID_NOT_LOCKED -2
#define SKETCHNET_ACK_ID_LOCKED		-3
*/
#define SKETCHNET_ACK_NOT_VALID		-4

typedef enum TOKENTYPE{ttCommand, ttOperator, ttPrimitive, ttNone};

#define DEFAULT_PORT_NUM 27003
#define MAX_LOG_SIZE 10

struct SettingsNetwork
{
	QStringList ips;
	quint16 port;
	bool bAutoConnect;
};

struct SKETCHCMDPACKET
{
	SKETCHCMD	 cmd;
	SKETCHACK	 ack;
	BlobNodeType opType;
	SkeletonType primType;
	int			 msgID;
	int			 blobnodeID;
	int			 leftChild;
	int			 rightChild;
	vec4f		 param;
};

	
class CMember : public QObject
{
	Q_OBJECT
private:
	bool m_bAlive;
	bool m_bLogSend;
	QStringList m_lstLogSend;
protected:
	QTcpSocket m_socket;
	quint16  m_port;
	quint64  m_bytesSent;
	DVec<SKETCHCMDPACKET> m_txPackets;
public:
	QString	 m_strAlias;
	QString  m_strAddress;	
	QTime	 m_lastActTime;
	size_t   m_ctSent;
	size_t   m_ctRecv;	
	size_t	 m_ctErrors;
public:
	CMember(const QString& strIPAddress, quint16 port = DEFAULT_PORT_NUM);

	~CMember()
	{
		if(m_bLogSend) saveLog();		
		m_lstLogSend.clear();
		if(m_socket.isValid())	
			m_socket.close();
	}
	void disconnect();
	void connectToHost();

	void setLog(bool bLogSend) {m_bLogSend =  bLogSend;}
	bool sendText(QString strText);
	bool sendPacket(SKETCHCMDPACKET& packet);
	int  recvAck(const SKETCHCMDPACKET& packet);
	void incrRecv() {m_ctRecv++;}
	
	bool isAlive() const { return m_bAlive;}
	bool isConnected() const {return m_socket.isValid();}
	void activate();

	int  getSentCount() const {return m_ctSent;}
	int  getRecvCount() const {return m_ctRecv;}
	int secondsInactive() const	{	return m_lastActTime.secsTo(QTime::currentTime());	}
private:
	void saveLog();
public slots:
	void actNetConnected();
	void actNetBytesWritten(quint64 written);
	void actNetDisplayError(QAbstractSocket::SocketError socketError);
signals:
	void upgrade();
	void downgrade();
};


//////////////////////////////////////////////////////////////////////////
// Manages members and Server connection
// Finds and Accesses members
//////////////////////////////////////////////////////////////////////////
class CDesignNet : public QObject
{
	Q_OBJECT
private:
	QTcpServer m_tcpServer;
	QTcpSocket* m_tcpServerConnection;
	DVec<CMember*> m_lstPending;
	DVec<CMember*> m_lstMembers;	
	quint16 m_port;
public:		
	CDesignNet(quint16 port = DEFAULT_PORT_NUM) { m_port = port;}
	~CDesignNet()
	{
		stop();
		removeAll();
	}

	bool sendMessageToMember(const QString& strDest, const QString& strMsg, bool bSendToAll = false);
	bool sendMessageToMember(const QString& strMsg, int idxMember = -1);
	bool sendPacketToMember(SKETCHCMDPACKET& packet, int idxMember = -1);

	void setPort(quint16 port) {m_port = port;}
	quint16 getPort() const {return m_port;}

	//Members
	size_t countMembers() const {return m_lstMembers.size();}
	CMember* getMember(int idxMember) const;
	bool addMember(const QString& strAddress);
	bool removeMember(const QString& strAddress);
	CMember* findMember(const QString& strAlias, int* pIndex = NULL, int* pIsPending = NULL);	
	bool removeMemberByName(const QString& strAlias);
	bool removeMember(CMember* a);
	void removeAll();
	QStringList getNames() const;
	QStringList getIPs() const;

	//Service
	bool start();
	void stop();
	
	static CDesignNet* GetDesignNet();
	

	signals:
		void sig_newMessage(int member, QString strMsg);
		void sig_memberslist(const QStringList& q);



	public slots:
		void actNetAcceptConnection();
		void actNetServerReadData();
		void actNetServerDisplayError(QAbstractSocket::SocketError socketError);
		int actNetUpgradeToMembers();
		int actNetDowngradeToPending();
protected:
	static CDesignNet* sm_pDesignNet;

};

//////////////////////////////////////////////////////////////////////////

/*
typedef const char * PS_ERROR;
PS_ERROR err_noerror	 = "No Error.";
PS_ERROR err_blobnode_id = "BlobNode ID not found.";
PS_ERROR err_msg_id		 = "Message ID not found.";
PS_ERROR err_param		 = "Param not found.";
PS_ERROR err_left_child  = "Left child not found.";
PS_ERROR err_right_child = "right child not found.";
PS_ERROR err_unknown	 = "Unknown Command.";
*/

class CSketchNetCommandTranslator
{
private:
	DAnsiStr m_strLastError;


protected:
	typedef enum PROTOCOL_ERROR{peNoError, peBlobNodeID, peMsgID, peParam, peLChild, peRChild, peUnknown};

	typedef std::map<string,int> TOKENS;
	typedef std::map<int,string> SIGNATURES;
	TOKENS		m_mapTokens;	
	SIGNATURES	m_mapTXCommands;
	SIGNATURES	m_mapTXPrims;
	SIGNATURES	m_mapTXOps;	
	SIGNATURES  m_mapTXAcks;
	SIGNATURES  m_mapErrors;

	
	DVec<DAnsiStr> m_keywords;		
	size_t m_DicOffsetOps;
	size_t m_DicOffsetPrims;
	static CSketchNetCommandTranslator* sm_pCmdTranslator;
public:
	CSketchNetCommandTranslator();
	~CSketchNetCommandTranslator();

	static CSketchNetCommandTranslator* GetCmdTranslator();
	bool translateStrToPacket(const QString& strInputCmd, SKETCHCMDPACKET& output);
	bool translatePacketToStr(const SKETCHCMDPACKET& inputCmd, QString& strOutput);

	DAnsiStr getLastError() const {return m_strLastError;}
private:	
	bool readInteger(DAnsiStr& strCmd, int& value);
	bool readParam(DAnsiStr& strCmd, vec4f& value);
	bool readString(DAnsiStr& strCmd, DAnsiStr& value);	
	TOKENTYPE getTokenType(const DAnsiStr& token) const;
};


}
#endif
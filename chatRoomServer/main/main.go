package main

import (
	"chatRoomServer/common"
	"chatRoomServer/utils"
	"database/sql"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
	"net"
	"time"
)

func main() {
	// 创建服务器
	listen, err := net.Listen("tcp", "0.0.0.0:8880")
	if err != nil {
		fmt.Println("创建服务器出现错误, 错误信息: ", err)
		return
	}
	// 关闭服务器
	defer listen.Close()

	if err != nil {
		fmt.Println("创建服务器出现错误, 错误信息: ", err)
		return
	}
	for {
		// 监听, 获取连接
		conn, err := listen.Accept()
		if err != nil {
			fmt.Println("服务器接收客户端连接服务器请求时出现错误, 错误信息: ", err)
			return
		}

		// 接收到连接
		// 开启协程
		go parseConnData(conn)
	}
}

// 解析连接得到的数据
func parseConnData(conn net.Conn) {
	// 设置读取和写入客户端数据的超时时间
	// time.Now().Add(time.Second * 10)超时时间10秒
	// time.Now()是当前时间, .Add(time.Second * 10)添加时间
	conn.SetDeadline(time.Now().Add(time.Second * 10))

	fmt.Println("接收到请求")
	open, err := sql.Open(common.DriverName, common.MysqlDriver)
	defer open.Close()

	var readData []byte = make([]byte, 10240)
	read, err := conn.Read(readData)
	if err != nil {
		return
	}

	var readString = string(readData[:read])
	// 解析
	analysis(readString, conn, open)
	conn.Close()
}

func analysis(readString string, conn net.Conn, open *sql.DB) {
	analysisString, m := utils.AnalysisString(readString)
	if analysisString == "login" {
		utils.Login(m, conn, open)
	} else if analysisString == "create" {
		utils.Create(m, conn, open)
	} else if analysisString == "updateContacts" {
		utils.UpdateContacts(m, conn, open)
	} else if analysisString == "UpdateContactsGroup" {
		utils.UpdateContactsGroup(m, conn, open)
	} else if analysisString == "sendMessageContact" {
		fmt.Println(m["message"])
		utils.SendMessageContact(m, conn, open)
	} else if analysisString == "sendMessageGroup" {
		utils.SendMessageGroup(m, conn, open)
	} else if analysisString == "getMessageBySendId" {
		utils.GetMessageBySendId(m, conn, open)
	} else if analysisString == "getGroupMessageBySendId" {
		utils.GetGroupMessageBySendId(m, conn, open)
	} else if analysisString == "addContact" {
		utils.AddContact(m, conn, open)
	} else if analysisString == "addGroup" {
		utils.AddGroup(m, conn, open)
	} else if analysisString == "upFileData" {
		utils.UpFileData(m, conn, open)
	} else if analysisString == "downloadFile" {
		utils.DownloadFile(m, conn, open)
	} else if analysisString == "getFileInfo" {
		utils.GetFileInfo(m, conn, open)
	}
}

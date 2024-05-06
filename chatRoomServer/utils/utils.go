package utils

import (
	"chatRoomServer/common"
	"database/sql"
	"fmt"
	uuid "github.com/satori/go.uuid"
	"io"
	"net"
	"os"
	"strconv"
	"strings"
)

// 解析数据函数
func AnalysisString(request string) (string, map[string]string) {
	var allData []string = strings.Split(request, "[")
	allData[1] = strings.Split(allData[1], "]")[0]
	var requestName string = allData[0]

	allData = strings.Split(allData[1], ",")
	var returnMap = make(map[string]string, 0)
	for i := 0; i < len(allData); i++ {
		var temp []string
		temp = strings.Split(allData[i], ":")
		returnMap[temp[0]] = temp[1]
	}

	return requestName, returnMap
}

func Login(str map[string]string, conn net.Conn, open *sql.DB) {

	var name string = str["name"]
	var password string = str["password"]

	// 查询
	query, err := open.Query("select name, password from " + common.GoMysqlUser + " where id = '" + name + "';")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				panic(err)
				return
			}
		}
	}()

	var sqlName string
	var sqlPassword string
	// 接收参数
	if query.Next() {
		err = query.Scan(&sqlName, &sqlPassword)
		if password == sqlPassword {
			_, err := conn.Write([]byte("type:login,isLogin:true,username:" + sqlName))
			if err != nil {
				fmt.Println("连接数据库出现错误, 错误信息: ", err)
				return
			}
			return
		}
	}

	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	_, err = conn.Write([]byte("type:login,isLogin:false"))
	if err != nil {
		return
	}
}

// 创建账户
func Create(str map[string]string, conn net.Conn, open *sql.DB) {
	// 生成uuid
	var id = uuid.NewV1().String()

	// 获取密码
	var password string = str["password"]
	var name string = str["name"]
	// 连接数据库, 将数据传入数据库中
	prepare, err := open.Prepare("insert into " + common.GoMysqlUser + "(id, password, name) value (?, ?, ?)")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	_, err = prepare.Exec(id, password, name)
	if err != nil {
		fmt.Println("执行sql语句时, 错误信息: ", err)
		return
	}

	// 返回id
	var returnData = "type:create,isCreate:true,id:" + id
	_, err = conn.Write([]byte(returnData))
	if err != nil {
		fmt.Println("返回数据信息时出现错误, 错误信息: ", err)
		return
	}
}

func UpdateContacts(str map[string]string, conn net.Conn, open *sql.DB) {
	// 查询
	var id = str["id"]
	query, err := open.Query("select contactId, messageId, contactName from " + common.GoMysqlContacts +
		" where userId = '" + id + "';")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				panic(err)
				return
			}
		}
	}()

	var contactId string
	var messageId string
	var contactName string
	var temp interface{}
	var returnMessage string = "type:UpdateContacts"
	i := 0
	for ; query.Next(); i++ {
		err = query.Scan(&contactId, &temp, &contactName)

		if temp != nil {
			messageIdI := temp.([]uint8)
			messageId = string(messageIdI)
		}
		if err != nil {
			fmt.Println("连接数据库出现错误, 错误信息: ", err)
			return
		}
		returnMessage += ",contact"
		returnMessage += strconv.Itoa(i)
		returnMessage += "contactId:" + contactId

		returnMessage += ",contact"
		returnMessage += strconv.Itoa(i)
		returnMessage += "userId:" + id

		returnMessage += ",contact"
		returnMessage += strconv.Itoa(i)
		returnMessage += "messageId:" + messageId

		returnMessage += ",contact"
		returnMessage += strconv.Itoa(i)
		returnMessage += "contactName:" + contactName
	}
	returnMessage += ",length:" + strconv.Itoa(i)

	_, err = conn.Write([]byte(returnMessage))
	if err != nil {
		return
	}
}

func UpdateContactsGroup(str map[string]string, conn net.Conn, open *sql.DB) {
	// 查询
	var id = str["id"]
	query, err := open.Query("select userId, groupId, groupName from `" + common.GoMysqlGroupUser +
		"` where userId = '" + id + "';")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				panic(err)
				return
			}
		}
	}()

	var userId string
	var groupName string
	var groupId string
	var returnMessage string
	i := 0
	for ; query.Next(); i++ {
		err = query.Scan(&userId, &groupId, &groupName)
		returnMessage += ",group"
		returnMessage += strconv.Itoa(i)
		returnMessage += "userId:" + userId

		returnMessage += ",group"
		returnMessage += strconv.Itoa(i)
		returnMessage += "groupId:" + groupId

		returnMessage += ",group"
		returnMessage += strconv.Itoa(i)
		returnMessage += "groupName:" + groupName
	}
	returnMessage += ",length:" + strconv.Itoa(i)

	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	_, err = conn.Write([]byte(returnMessage))
	if err != nil {
		return
	}
}

func SendMessageContact(str map[string]string, conn net.Conn, open *sql.DB) {
	// 先查询出该用户与联系人的消息, 在将发送的消息添加到查询出的消息上,
	// 如果查询出的消息超过1000字符串重置消息, 再将消息发送到数据库中

	// 查询消息
	var contactId string = str["id"]
	var userId string = str["sendUser"]
	query, err := open.Query("select messageId, contactId, userId, message from `" + common.GoMysqlContactMessage +
		"` where contactId = '" + contactId + "' and userId = '" + userId + "' or contactId = '" +
		userId + "' and userId = '" + contactId + "';")

	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				panic(err)
				return
			}
		}
	}()

	var messageId string
	var message string

	if query.Next() {
		// 数据库中有该数据
		err = query.Scan(&messageId, &contactId, &userId, &message)

		if len(message) >= 200 {
			message = str["sendUser"] + ":" + str["message"]
		} else {
			message += "," + str["sendUser"] + ":" + str["message"]
		}

		// 发送数据回客户端
		_, err = conn.Write([]byte("returnUserMessage[message[" + message + "],sendId:" + contactId + ",userId:" + userId + "]"))
		if err != nil {
			return
		}

		// 将数据库中的数据更新
		prepare, err := open.Prepare("update `" + common.GoMysqlContactMessage + "` set message = ? where messageId = ?")
		if err != nil {
			fmt.Println("连接数据库出现错误, 错误信息: ", err)
			return
		}

		_, err = prepare.Exec(message, messageId)
		if err != nil {
			fmt.Println("执行sql语句时, 错误信息: ", err)
			return
		}
		fmt.Println("returnUserMessage[message[" + message + "],sendId:" + contactId + ",userId:" + userId + "]")
	} else {
		// 没有, 新建
		// 发送数据回客户端
		if len(message) > 100 {
			message = str["sendUser"] + ":" + str["message"]
		} else {
			message += "," + str["sendUser"] + ":" + str["message"]
		}

		_, err = conn.Write([]byte("returnUserMessage[message[" + message + "],sendId:" + contactId + ",userId:" + userId + "]"))
		if err != nil {
			return
		}

		prepare, err := open.Prepare("insert into " + common.GoMysqlContactMessage + "(messageId, contactId, userId, message) value (?, ?, ?, ?)")
		if err != nil {
			fmt.Println("连接数据库出现错误, 错误信息: ", err)
			return
		}
		messageId = uuid.NewV1().String()
		_, err = prepare.Exec(messageId, contactId, userId, message)
		if err != nil {
			fmt.Println("执行sql语句时, 错误信息: ", err)
			return
		}
		fmt.Println(message)

		prepare, err = open.Prepare("update `" + common.GoMysqlContacts +
			"` set messageId = ? where contactId = ? and userId = ? or userId = ? and contactId = ?")

		if err != nil {
			fmt.Println("连接数据库出现错误, 错误信息: ", err)
			return
		}

		_, err = prepare.Exec(messageId, contactId, userId, contactId, userId)
		if err != nil {
			fmt.Println("执行sql语句时, 错误信息: ", err)
			return
		}
	}
}

func SendMessageGroup(str map[string]string, conn net.Conn, open *sql.DB) {
	// 查询用户消息
	var groupId = str["id"]
	var sendUser = str["sendUser"]
	var sendMessage = str["message"]
	query, err := open.Query("select groupMessage from `" + common.GoMysqlGroup +
		"` where groupId = '" + groupId + "';")

	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				panic(err)
				return
			}
		}
	}()

	var groupMessage string
	if query.Next() {
		query.Scan(&groupMessage)
		if len(groupMessage) >= 200 {
			groupMessage += sendUser + ":" + sendMessage
		} else {
			groupMessage += "," + sendUser + ":" + sendMessage
		}
	}
	prepare, err := open.Prepare("update `" + common.GoMysqlGroup + "` set groupMessage = ? where groupId = ?")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	_, err = prepare.Exec(groupMessage, groupId)
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}
	groupMessage = "returnUserMessage[message[" + groupMessage + "]]"
	_, err = conn.Write([]byte(groupMessage))
	if err != nil {
		fmt.Println("返回数据出现错误, 错误信息: ", err)
		return
	}
}

func GetGroupMessageBySendId(str map[string]string, conn net.Conn, open *sql.DB) {
	// 查询用户消息
	var groupId = str["groupId"]
	query, err := open.Query("select groupMessage from `" + common.GoMysqlGroup +
		"` where groupId = '" + groupId + "';")

	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				panic(err)
				return
			}
		}
	}()

	var message string
	var returnData string = ""
	if query.Next() {
		query.Scan(&message)
		returnData += message
	}

	_, err = conn.Write([]byte(returnData))
	if err != nil {
		fmt.Println("返回数据出现错误, 错误信息: ", err)
		return
	}
}

func GetMessageBySendId(str map[string]string, conn net.Conn, open *sql.DB) {
	// 查询用户消息
	var messageId = str["messageId"]
	query, err := open.Query("select message from `" + common.GoMysqlContactMessage +
		"` where messageId = '" + messageId + "';")

	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				panic(err)
				return
			}
		}
	}()

	var message string
	var returnData string = ""
	if query.Next() {
		query.Scan(&message)
		returnData += message
	}

	_, err = conn.Write([]byte(returnData))
	if err != nil {
		return
	}
}

func AddGroup(str map[string]string, conn net.Conn, open *sql.DB) {
	// 查询用户消息
	var id = str["id"]
	var name = str["name"]
	query, err := open.Query("select groupName from `" + common.GoMysqlGroup +
		"` where groupId = '" + name + "';")

	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				fmt.Println(err)
				panic(err)
				return
			}
		}
	}()

	if query.Next() {
		var groupName string
		query.Scan(&groupName)
		prepare, err := open.Prepare("insert into " + common.GoMysqlGroupUser + "(userId, groupId, groupName) values (?, ?, ?)")
		_, err = prepare.Exec(id, name, groupName)
		if err != nil {
			fmt.Println("添加数据库错误, err: ", err)
			return
		}
		_, err = conn.Write([]byte("添加成功" + name))
		if err != nil {
			fmt.Println("返回数据错误, err: ", err)
			return
		}
	} else {
		// 添加
		prepare, err := open.Prepare("insert into `" + common.GoMysqlGroup + "`(groupId, groupName, groupMessage) values (?, ?, ?)")
		if err != nil {
			fmt.Println("添加数据库错误, err: ", err)
			return
		}
		var groupId = uuid.NewV1().String()
		_, err = prepare.Exec(groupId, name, "")
		if err != nil {
			fmt.Println("添加数据库错误, err: ", err)
			return
		}
		_, err = conn.Write([]byte(groupId))
		if err != nil {
			fmt.Println("返回数据错误, err: ", err)
			return
		}

		prepare, err = open.Prepare("insert into `" + common.GoMysqlGroupUser + "`(userId, groupId, groupName) values (?, ?, ?)")
		_, err = prepare.Exec(id, groupId, name)
		if err != nil {
			fmt.Println("添加数据库错误, err: ", err)
			return
		}
		_, err = conn.Write([]byte("添加成功" + groupId))
		if err != nil {
			fmt.Println("返回数据错误, err: ", err)
			return
		}
	}
}

func AddContact(str map[string]string, conn net.Conn, open *sql.DB) {
	var id = str["id"]

	begin, _ := open.Begin()
	var addContactId = str["addContactId"]
	fmt.Println(addContactId)
	fmt.Println(str["addContactId"])
	query, err := open.Query("select contactId, userId from `" + common.GoMysqlContacts +
		"` where contactId = '" + id + "' and userId = '" + addContactId + "' or contactId = '" +
		addContactId + "' and userId = '" + id + "';")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				panic(err)
				return
			}
		}
	}()
	//panic(errors.New("err").Error())

	if query.Next() {
		// 判断数据库中是否已有数据
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	var idName string
	var addIdName string

	query, err = open.Query("select name from `" + common.GoMysqlUser +
		"` where id = '" + id + "';")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}

	if query.Next() {
		query.Scan(&idName)
	}

	query, err = open.Query("select name from `" + common.GoMysqlUser +
		"` where id = '" + addContactId + "';")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		return
	}
	if query.Next() {
		query.Scan(&addIdName)
	}

	// 将数据库中的数据更新
	prepare, err := open.Prepare("insert into " + common.GoMysqlContactMessage +
		"(messageId, contactId, userId, message) value (?, ?, ?, ?)")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		begin.Rollback()
		return
	}

	v1 := uuid.NewV1().String()
	_, err = prepare.Exec(v1, id, addContactId, "")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		begin.Rollback()
		return
	}

	// 存数据
	prepare, err = open.Prepare("insert into " + common.GoMysqlContacts +
		"(contactId, userId, messageId, contactName) value (?, ?, ?, ?)")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		begin.Rollback()
		return
	}
	_, err = prepare.Exec(id, addContactId, v1, idName)
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		begin.Rollback()
		return
	}

	prepare, err = open.Prepare("insert into " + common.GoMysqlContacts +
		"(contactId, userId, messageId, contactName) value (?, ?, ?, ?)")
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		begin.Rollback()
		return
	}
	_, err = prepare.Exec(addContactId, id, v1, addIdName)
	if err != nil {
		fmt.Println("连接数据库出现错误, 错误信息: ", err)
		begin.Rollback()
		return
	}
	begin.Commit()

	_, err = conn.Write([]byte("succeed"))
	if err != nil {
		return
	}
}

// 上传文件
// 一共上传两次数据, 第一次注册信息到数据库中,第二次将数据存储到文件中

// 上传文件
// 保存文件到电脑中
func UpFileData(str map[string]string, conn net.Conn, open *sql.DB) {
	// 注册信息
	strs := strings.Split(str["fileName"], "/")
	prepare, err := open.Prepare("insert into " + common.GoMysqlFile + " (fileName, userId, fileId) value (?, ?, ?)")
	if err != nil {
		fmt.Println(err)
		return
	}
	v1 := uuid.NewV1()
	_, err = prepare.Exec(strs[len(strs)-1], str["saveUserId"], v1.String())
	if err != nil {
		fmt.Println(err)
		return
	}

	defer func() {
		if prepare != nil {
			err := prepare.Close()
			if err != nil {
				return
			}
		}
	}()

	defer func() {
		if prepare != nil {
			err := prepare.Close()
			if err != nil {
				panic(err)
				return
			}
		}
	}()

	_, err = conn.Write([]byte("ok"))
	if err != nil {
		fmt.Println(err)
		return
	}
	var data []byte = make([]byte, 1024)
	//fmt.Println(data[:read])
	getwd, err := os.Getwd()
	fmt.Println(getwd)
	// 保存文件到服务器
	file, err := os.OpenFile(getwd+"\\data\\"+v1.String()+strs[len(strs)-1], os.O_CREATE|os.O_APPEND, 0777)
	for {
		read, _ := conn.Read(data)
		if read == 2 && string(data[:read]) == "ok" {
			break
		}
		file.Write(data[:read])
		conn.Write([]byte("ok"))
	}
	_, err = conn.Write([]byte("id:" + v1.String()))
	fmt.Println("id:" + v1.String())
	if err != nil {
		fmt.Println(err)
		return
	}
	file.Close()
}

// 下载文件
func DownloadFile(str map[string]string, conn net.Conn, open *sql.DB) {
	name := str["name"]
	getwd, err := os.Getwd()
	if err != nil {
		fmt.Println(err)
		return
	}
	file, err := os.OpenFile(getwd+"\\data\\"+name, os.O_RDONLY, 0777)
	if err != nil {
		fmt.Println(err)
		return
	}
	var data []byte = make([]byte, 1024)
	var i int64 = 0
	for {
		read, err := file.ReadAt(data, i)
		var num = int64(read)
		i += num
		if err != nil {
			if err == io.EOF {
				_, err = conn.Write(data[:read])
				break
			}
			fmt.Println(err)
			return
		}
		_, err = conn.Write(data[:read])
		if err != nil {
			fmt.Println(err)
			return
		}
	}
	conn.Read(data)
	_, err = conn.Write([]byte("ok"))
	if err != nil {
		fmt.Println(err)
		return
	}
}

func GetFileInfo(str map[string]string, conn net.Conn, open *sql.DB) {
	// 获取可下载的文件信息
	// 检索数据库
	query, err := open.Query("select fileName, fileId from " + common.GoMysqlFile + " where userId = '" + str["userId"] + "'")
	if err != nil {
		fmt.Println(err)
		return
	}

	defer func() {
		if query != nil {
			err := query.Close()
			if err != nil {
				fmt.Println(err)
				return
			}
		}
	}()

	var data = ""
	var fileName string
	var fileId string
	i := 0
	for query.Next() {
		query.Scan(&fileName, &fileId)
		if data == "" {
			data += "file" + strconv.Itoa(i) + ":" + fileId + fileName
			i++
			continue
		}
		data += ",file" + strconv.Itoa(i) + ":" + fileId + fileName
		i++
	}
	_, err = conn.Write([]byte(data))
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println(data)
}

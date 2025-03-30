

/**
 * ssh连接状态
 */
enum yxulinux_connect_state {
  YXU_CONNECT_UNKNOWN = 0,          // 未初始化状态
  YXU_CONNECT_CREATED_SESSION,      // 已创建session
  YXU_CONNECT_CONNECTED,            // 已连接
  YXU_CONNECT_VERIFY_HOST,          // 已验证主机
  YXU_CONNECT_AUTH,                 // 完成验证
};

/**
 * 文件类型
 */
const yxulinux_file_type = {
  YXU_S_IFMT:    "017",
  YXU_S_IFSOCK:  "014",    // 套接字
  YXU_S_IFLNK:   "012",    // 符号链接
  YXU_S_IFREG:   "010",    // 普通文件
  YXU_S_IFBLK:   "006",    // 块设备
  YXU_S_IFDIR:   "004",    // 文件夹
  YXU_S_IFCHR:   "002",    // 字符设备
  YXU_S_IFIFO:   "001",    // 命名管道	
};


enum yxulinux_error_code {
  YXU_UNKNOWN=0,                                    // 未知状态
  YXU_SUCCESS,                                      // 成功
  YXU_EMPTY,                                        // 空
  YXU_READ_SUCCESS,                                 // 成功
  YXU_OPERATION_FAILED = 100,                       // 操作失败
  YXU_PARAMETER_ERROR,                              // 参数错误
  YXU_CREATE_SESSION_FAILED,                        // ssh创建session失败
  YXU_HOST_EMPTY,                                   // 主机地址为空
  YXU_PORT_ERROR,                                   // 端口错误
  YXU_CONNECT_FAILED,                               // 连接失败
  YXU_GET_SERVER_PUBLIC_KEY_FAILED,                 // 获取服务器的public_key失败
  YXU_KNOWN_HOSTS_CHANGED,                          // ssh: 服务器的主机密钥已更改，出于安全原因，连接将停止
  YXU_KNOWN_HOSTS_OTHER,                            // ssh: 未找到此服务器的主机密钥，但存在其他类型的密钥。
  YXU_KNOWN_HOSTS_NOT_FOUND,                        // ssh: 无法找到已知主机文件。
  YXU_KNOWN_HOSTS_UNKNOWN,                          // ssh: 服务器未知，您是否信任主机密钥？(需要弹窗提醒用户)
  YXU_KNOWN_HOSTS_ERROR,                            // ssh: 验证主机错误
  YXU_UPDATE_KNOWN_HOST_FAILED,                     // 更新信任主机密钥失败
  YXU_AUTH_WITH_USER_FAILED,                        // 认证失败，使用密码登录方式失败，可能是密码错误
  YXU_AUTH_WITH_PRIVATE_KEY_FAILED,                 // 认证失败，使用PrivateKey登录方式失败
  YXU_IMPORT_PRIVATE_KEY_FAILED,                    // 认证失败，导入PrivateKey失败
  YXU_AUTH_WITH_PUBLIC_KEY_FAILED,                  // 认证失败，导入PublicKey失败
  YXU_HOST_NOT_SUPPORT_PUBLIC_KEY_AUTH,             // 认证失败，主机不支持公钥认证
  YXU_GENERATE_CHANNEL_FAILED,                      // 生成频道失败
  YXU_OPEN_CHANNEL_FAILED,                          // 打开频道失败
  YXU_READ_CHANNEL_FAILED,                          // 读取频道消息失败
  YXU_WRITE_CHANNEL_FAILED,                         // 写入频道消息失败
  YXU_RUN_COMMAND_FAILED,                           // 执行命令失败
  YXU_OPEN_DIR_FAILED,                              // 目录打开失败
  YXU_READ_DIR_FAILED,                              // 目录读取失败
  YXU_CLOSE_DIR_FAILED,                             // 目录关闭失败
  YXU_SFTP_ERROR,                                   // sftp获取文件信息错误
  YXU_FX_EOF,                                       // End-of-file encountered，遇到文件结尾
  YXU_FX_NO_SUCH_FILE,                              // File doesn't exist  文件不存在
  YXU_FX_PERMISSION_DENIED,                         // Permission denied  权限拒绝
  YXU_FX_FAILURE,                                   // Generic failure， 通用错误
  YXU_FX_BAD_MESSAGE,                               // Garbage received from server， 从服务器接收到垃圾数据
  YXU_FX_NO_CONNECTION,                             // No connection has been set up , 没有建立连接
  YXU_FX_CONNECTION_LOST,                           // 连接丢失
  YXU_FX_OP_UNSUPPORTED,                            // Operation not supported by the server， 服务器不支持当前操作
  YXU_FX_INVALID_HANDLE,                            // 无效的文件句柄
  YXU_FX_NO_SUCH_PATH,                              // 没有搜索到文件或目录
  YXU_FX_FILE_ALREADY_EXISTS,                       // 文件已经存在
  YXU_FX_WRITE_PROTECT,                             // 尝试写一个写保护文件系统
  YXU_FX_NO_MEDIA,                                  // 远程驱动器中没有媒体
  YXU_NO_WRITE_PERMISSION,                          // 没有写权限
  YXU_OPERATION_SERVER_FILE_FAILED,                 // 操作服务器文件失败
  YXU_OPERATION_LOCAL_FILE_FAILED,                  // 操作本地文件失败
  YXU_FILE_TOO_LARGE,                               // 文件太大，不建议使用编辑器打开
  YXU_READ_FILE_FAILED,                             // 读取文件失败
  YXU_WRITE_FILE_FAILED,                            // 写入文件失败
  YXU_CREATE_CHANNEL_FAILED,                        // 创建channel失败
  YXU_SSH_READ_FAILED,                              // 读取数据失败
  YXU_AGENT_CHANNEL_FAILED,                         // 使用代理认证失败
  YXU_OPEN_FORWARD_FAILED,                          // 打开隧道失败
  YXU_NOT_A_DIRECTORY,                              // 不是一个目录
  YXU_NO_READ_PERMISSION,                           // 没有读权限
};



export {
  yxulinux_connect_state,
  yxulinux_file_type,
  yxulinux_error_code
}
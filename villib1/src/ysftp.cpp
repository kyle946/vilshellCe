#include "./ysftp.h"



int ysftp::generate()
{
  int ret;
  if(this->conn->connected==YXU_CONNECT_UNKNOWN){
    /// 1.
    ret = this->conn->create();
    if (ret != YXU_SUCCESS)
    {
      return YXU_CREATE_SESSION_FAILED;
    }
    /// 2.
    ret = this->conn->connnect();
    if (ret != YXU_SUCCESS)
    {
      return ret;
    }
    /// 3.
    ret = this->conn->authhost();
    if (ret != YXU_SUCCESS)
    {
      return ret;
    }
  }
  /// 4. 创建sftp会话
  sftp = sftp_new(this->conn->session);
  if (sftp == NULL)
  {
    return YXU_GENERATE_CHANNEL_FAILED;
  }
  ret = sftp_init(sftp);
  if(ret==SSH_OK){
    return YXU_SUCCESS;
  }else{
    release();
    return YXU_OPEN_CHANNEL_FAILED;
  }
}

int ysftp::release()
{
  if(sftp!=nullptr){
    sftp_free(sftp);
  }
  if(this->conn!=nullptr){
    this->conn->release();
  }
  return YXU_SUCCESS;
}


int ysftp::listdir(std::string path, JsonObj &arr)
{
  /// 判断目标路径是否存在，并且有读写权限
  sftp_attributes _attr = NULL;
  int rc = this->getpermission(*this->conn->uid, path, _attr);
  if(rc!=YXU_SUCCESS){
    return rc;
  }

  sftp_dir dir = sftp_opendir(sftp, path.c_str());
  if (!dir)
  {
    return YXU_OPERATION_FAILED;
  }
  sftp_attributes attributes;
  std::map<std::string, JsonObj> mapfile;
  std::map<std::string, JsonObj> mapdir;
  while ((attributes = sftp_readdir(sftp, dir)) != NULL)
  {
    if (strcmp(attributes->name, ".") == 0 || strcmp(attributes->name, "..") == 0)
    {
      continue;
    }

    JsonObj fileattr;
    std::string linkfilepath;
    std::string filetype;
    int islink = 0;
    std::string filepath = path + "/" + attributes->name;

    if ((attributes->permissions & SSH_S_IFLNK) == SSH_S_IFLNK)
    {
      // vdlog::getInstance()->info("name: %s, type: %s\n", attributes->name, "link");
      islink = 1;
      char *_buffer = sftp_readlink(sftp, filepath.c_str());
      if (_buffer == NULL)
      {
        vdlog::getInstance()->error("Invalid link: %s", attributes->name);
        islink = -1;
      }
      else
      {
        ssh_string_free_char(_buffer);
        std::string linkfile(_buffer);
        if (linkfile.find_first_of("/") == 0)
        {
          linkfilepath = linkfile;
        }
        else
        {
          linkfilepath = path + "/" + linkfile;
        }
        // vdlog::getInstance()->info("linkfile: %s\n", linkfile.c_str());
        // vdlog::getInstance()->info("linkfilepath: %s\n", linkfilepath.c_str());
        sftp_attributes _attr = sftp_stat(sftp, linkfilepath.c_str());
        if (_attr == NULL)
        {
          // Invalid link
          vdlog::getInstance()->error("Invalid link: %s", linkfile.c_str());
          islink = -1;
        }
        else
        {
          if ((_attr->permissions & SSH_S_IFREG) == SSH_S_IFREG)
          {
            // vdlog::getInstance()->info("link name: %s, type: %s\n", attributes->name, "file");
            filetype = "file";
          }
          if ((_attr->permissions & SSH_S_IFDIR) == SSH_S_IFDIR)
          {
            // vdlog::getInstance()->info("link name: %s, type: %s\n", attributes->name, "dir");
            filetype = "dir";
          }
        }
      }
    }
    else
    {
      if ((attributes->permissions & SSH_S_IFREG) == SSH_S_IFREG)
      {
        // vdlog::getInstance()->info("name: %s, type: %s\n", attributes->name, "file");
        filetype = "file";
      }
      if ((attributes->permissions & SSH_S_IFDIR) == SSH_S_IFDIR)
      {
        // vdlog::getInstance()->info("name: %s, type: %s\n", attributes->name, "dir");
        filetype = "dir";
      }
    }

    //--
    fileattr.Add("linkfilepath", linkfilepath);
    fileattr.Add("filepath", filepath);
    fileattr.Add("islink", islink);
    fileattr.Add("filetype", filetype);
    fileattr.Add("name", attributes->name);
    fileattr.Add("longname", attributes->longname);
    fileattr.Add("flags", attributes->flags);
    fileattr.Add("select", 0);
    fileattr.Add("type", attributes->type);
    fileattr.Add("size", attributes->size);
    fileattr.Add("uid", attributes->uid);
    fileattr.Add("gid", attributes->gid);
    fileattr.Add("owner", attributes->owner);
    fileattr.Add("group", attributes->group);
    fileattr.Add("permissions", attributes->permissions);
    fileattr.Add("atime64", attributes->atime64);
    fileattr.Add("atime", attributes->atime);
    fileattr.Add("atime_nseconds", attributes->atime_nseconds);
    fileattr.Add("createtime", attributes->createtime);
    fileattr.Add("createtime_nseconds", attributes->createtime_nseconds);
    fileattr.Add("mtime64", attributes->mtime64);
    fileattr.Add("mtime", attributes->mtime);
    fileattr.Add("mtime_nseconds", attributes->mtime_nseconds);
    fileattr.Add("extended_count", attributes->extended_count);
    if (filetype == "dir")
    {
      mapdir[attributes->name] = fileattr;
    }
    else
    {
      mapfile[attributes->name] = fileattr;
    }
  }
  for (auto [key, attr] : mapdir)
  {
    arr.Add(attr);
  }
  for (auto [key, attr] : mapfile)
  {
    arr.Add(attr);
  }
  if (!sftp_dir_eof(dir))
  {
    vdlog::getInstance()->error("Can't list directory: %s\n", ssh_get_error(this->conn->session));
    sftp_closedir(dir);
    return YXU_READ_DIR_FAILED;
  }
  rc = sftp_closedir(dir);
  if (rc != SSH_OK)
  {
    vdlog::getInstance()->error("Can't close directory: %s\n", ssh_get_error(this->conn->session));
    return YXU_CLOSE_DIR_FAILED;
  }
  return YXU_SUCCESS;
}

/**
 * @brief 从本地上传文件至服务器
 *
 * @param files   要上传文件或文件夹
 * @param destination_path    服务器目标路径
 * @param ptr_progress
 * @param ptr_numbers
 * @return int
 */
void ysftp::uploadfiles(std::vector<std::string> files,
                               std::string source_path,
                               std::string destination_path,
                               std::function<void(std::string filename, int size, int count)> ptr_progress,
                               std::function<void(int num, int nums, int code)> ptr_numbers)
{
  this->localfiles.clear();

  // 查找所有需要上传的文件
  for (std::string file : files)
  {
    std::string fpath = source_path + DIRSEPARATOR + file;
    this->uploadfiles_recursion_folder_local(fpath, 1);
  }
  int filenumber = 0;

  // 转换路径
  std::string _source_path;
  if (source_path.substr(source_path.size() - 1, 1) == DIRSEPARATOR)
  {
    _source_path = source_path.substr(0, source_path.size() - 1);
  }
  else
  {
    _source_path = source_path;
  }
  std::string _destination_path;
  if (destination_path.substr(destination_path.size() - 1, 1) == "/")
  {
    _destination_path = destination_path.substr(0, destination_path.size() - 1);
  }
  else
  {
    _destination_path = destination_path;
  }

  /// 判断目标路径是否存在，并且有读写权限
  {
    sftp_attributes _attr = sftp_stat(sftp, _destination_path.c_str());
    bool _exists = false;
    if(_attr==NULL){
      if (sftp_get_error(sftp) == SSH_FX_NO_SUCH_FILE) {
        // printf("文件不存在\n");
        ptr_numbers(-1, -1, YXU_FX_NO_SUCH_PATH);
        return;
      } else {
        // printf("无法获取文件属性\n");
        ptr_numbers(-1, -1, YXU_OPERATION_FAILED);
        return;
      }
    }else{
      // 文件存在
      if ((_attr->permissions & 0400) && (_attr->permissions & 0200)) {
        // printf("文件具有所有者读写权限\n");
      } else {
        // printf("文件不具有所有者读写权限\n");
        ptr_numbers(-1, -1, YXU_NO_WRITE_PERMISSION);
        return;
      }
    }
  }

  // 开始上传
  for (map1 field : this->localfiles)
  {

    std::string localfile = field["name"];
    std::string ff = common::replace(localfile, _source_path, "");
    std::string serverfile = _destination_path + "/" + ff;
    serverfile = common::replace(serverfile, "\\", "/");
    serverfile = common::replace(serverfile, "//", "/");

    // vdlog::getInstance()->info("server: %s", serverfile.c_str());
    // vdlog::getInstance()->info("local: %s", localfile.c_str());
    // vdlog::getInstance()->info("type: %s", field["type"].c_str());

    if (field["type"] == YXU_S_IFDIR_CHAR)
    {
      sftp_attributes _attr = sftp_stat(sftp, serverfile.c_str());
      bool _exists = false;
      if(_attr==NULL){
        if (sftp_get_error(sftp) == SSH_FX_NO_SUCH_FILE) {
          // printf("文件不存在\n");
          _exists = false;
        } else {
          // printf("无法获取文件属性\n");
          ptr_numbers(-1, -1, YXU_OPERATION_FAILED);
          return;
        }
      }else{
        // 文件存在
        _exists = true;
      }

      if(_exists){  // 如果文件夹存在
        if ((_attr->permissions & 0400) && (_attr->permissions & 0200)) {
          // printf("文件具有所有者读写权限\n");
        } else {
          // printf("文件不具有所有者读写权限\n");
          ptr_numbers(-1, -1, YXU_NO_WRITE_PERMISSION);
          return;
        }
      }else{  // 如果文件夹不存在
        common::_mkdir(serverfile);
        int rc = sftp_mkdir(sftp, serverfile.c_str(), 0777);
        if (rc == SSH_OK)
        {
        }
        else
        {
          ptr_numbers(-1, -1, YXU_WRITE_FILE_FAILED);
          return;
        }
      }
    }
    else
    {
      // int access_type;
      char buffer[MAX_XFER_BUF_SIZE];
      int nbytes, nwritten, rc, fc;

      // 打开本地文件
      FILE *fd = common::_fopen(localfile, "rb");
      if (fd==nullptr)
      {
        vdlog::getInstance()->error("Can't open file for writing: %s\n", localfile.c_str());
        ptr_numbers(-1, -1, YXU_OPERATION_LOCAL_FILE_FAILED);
        return;
      }

      // 打开服务器文件
      sftp_file file = sftp_open(sftp, serverfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
      if (file == NULL)
      {
        vdlog::getInstance()->error("Can't open file for reading: %s\n", ssh_get_error(this->conn->session));
        int ret1 = this->geterror();
        ptr_numbers(-1, -1, ret1);
        return;
      }
      
      ptr_numbers(filenumber, this->localfiles.size(), YXU_EMPTY);

      // 读取本地文件的数据 ，向服务器文件写入
      int wsize = 0;
      uint64_t _now = 0;
      for (;;)
      {

        this->transferlock->lock();
        if (*this->stoptransfer == 1)
        {
          this->transferlock->unlock();
          vdlog::getInstance()->info("stop transfer.");
          ptr_numbers(-1, -1, YXU_OPERATION_FAILED);
          return;
        }
        else
        {
          this->transferlock->unlock();
        }

        nbytes = fread(buffer, sizeof(char), sizeof(buffer), fd);
        if (nbytes == 0)
        {
          ptr_progress(ff, wsize, std::stoi(field["size"]));
          break; // EOF
        }
        else if (nbytes < 0)
        {
          vdlog::getInstance()->error("Error while reading file: %s\n", strerror(errno));
          sftp_close(file);
          fclose(fd);
          ptr_numbers(-1, -1, YXU_OPERATION_LOCAL_FILE_FAILED);
          return;
        }
        nwritten = sftp_write(file, buffer, nbytes);
        if (nwritten != nbytes)
        {
          vdlog::getInstance()->error("Error writing: %s\n", ssh_get_error(this->conn->session));
          sftp_close(file);
          fclose(fd);
          ptr_numbers(-1, -1, YXU_OPERATION_SERVER_FILE_FAILED);
          return;
        }
        // 回调函数
        wsize = wsize + nbytes;
        // 防止回调函数调用过于频繁，导致主线程阻塞，这里每隔100毫秒调用一次
	      uint64_t timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()); // 获取时间戳（毫秒）
        // vdlog::getInstance()->warn("time: %lld", timestamp);
        if(timestamp>(_now+100)){
          ptr_progress(ff, wsize, std::stoi(field["size"]));
          _now = timestamp; 
        }
      }
      // 关闭打开的文件
      fc = fclose(fd);
      rc = sftp_close(file);
    }
    // 回调函数
    filenumber = filenumber + 1;
    ptr_numbers(filenumber, this->localfiles.size(), YXU_EMPTY);
  }
  vdlog::getInstance()->warn("end of thread.(uploadfile)");
}



/**
 * @brief 遍历本地目录 ，准备上传
 *
 * @param _dir 要递归的目录
 * @param mode 1 外部调用 ，2 递归函数调用
 * @return int
 */
int ysftp::uploadfiles_recursion_folder_local(std::string _dir, int mode)
{
  fstat1 f_attr = common::stat1(_dir);
  if (strcmp(f_attr.type, YXU_S_IFDIR_CHAR) == 0)
  {
    map1 field;
    field["name"] = _dir;
    field["type"] = YXU_S_IFDIR_CHAR;
    this->localfiles.push_back(field);
    //--
    std::string dirstr = _dir.substr(_dir.size() - 1, 1) == DIRSEPARATOR ? _dir.substr(0, _dir.size() - 1) : _dir;
    #ifdef _WIN32
    _WDIR *odir = _wopendir(common::string_to_wstring(dirstr).c_str());
    #else
    DIR *odir = opendir(dirstr.c_str());
    #endif
    if (odir == NULL)
    {
      return YXU_SFTP_ERROR;
    }

    #ifdef _WIN32
    _wdirent *drt = nullptr;
    while ((drt = _wreaddir(odir)) != NULL)
    #else
    dirent *drt = nullptr;
    while ((drt = readdir(odir)) != NULL)
    #endif
    {
      #ifdef _WIN32
      std::wstring d_name = std::wstring(drt->d_name);
      std::string d_name_ = common::wstring_to_string(d_name);
      #else
      std::string d_name_(drt->d_name);
      #endif
      if (drt->d_type == DT_DIR)
      {
        if (d_name_ == "." || d_name_ == "..")
        {
          continue;
        }
        std::string childpath = dirstr + DIRSEPARATOR + d_name_;
        vdlog::getInstance()->info("dirpath: %s", childpath.c_str());
        this->uploadfiles_recursion_folder_local(childpath, 2);
      }
      else if (drt->d_type == DT_REG)
      {
        std::string filename = dirstr + DIRSEPARATOR + d_name_;
        vdlog::getInstance()->info("filename: %s", filename.c_str());
        map1 field;
        field["name"] = filename;
        field["type"] = YXU_S_IFREG_CHAR;
        fstat1 _attr = common::stat1(filename);
        field["size"] = std::to_string(_attr.size);
        this->localfiles.push_back(field);
      }
      else
      {
        vdlog::getInstance()->error("file type error: %s, type: %d.", drt->d_name, drt->d_type);
      }
    }
    #ifdef _WIN32
    int rc = _wclosedir(odir);
    #else
    int rc = closedir(odir);
    #endif
  }
  else if (strcmp(f_attr.type, YXU_S_IFREG_CHAR) == 0)
  {
    map1 field;
    field["name"] = _dir;
    field["type"] = f_attr.type;
    field["size"] = std::to_string(f_attr.size);
    this->localfiles.push_back(field);
  }
  else
  {
    // vdlog::getInstance()->error("file type error: %s, type: %s.", f_attr.name, f_attr.type);
  }
  return YXU_SUCCESS;
}

/**
 * @brief 从服务器下载文件至本地
 *
 * @param files 要下载的文件或文件夹
 * @param destination_path    本地目标路径
 * @param ptr_progress
 * @param ptr_numbers
 * @return int
 */
void ysftp::downloadfiles(std::vector<std::string> files,
                                 std::string source_path,
                                 std::string destination_path,
                                 std::function<void(std::string filename, int size, int count)> ptr_progress,
                                 std::function<void(int num, int nums, int code)> ptr_numbers)
{

  /// 查找所有需要下载的文件
  this->serverfiles.clear();
  for (const std::string file : files)
  {
    this->downloadfiles_recursion_folder_server(source_path + "/" + file, 1);
  }

  int filenumber = 0;
  std::string _source_path;
  if (source_path.substr(source_path.size() - 1, 1) == "/")
  {
    _source_path = source_path.substr(0, source_path.size() - 1);
  }
  else
  {
    _source_path = source_path;
  }

  std::string _destination_path;
  if (destination_path.substr(destination_path.size() - 1, 1) == DIRSEPARATOR)
  {
    _destination_path = destination_path.substr(0, destination_path.size() - 1);
  }
  else
  {
    _destination_path = destination_path;
  }

  for (map1 field : this->serverfiles)
  {
    // sftp_open
    // sftp_read
    // sftp_write
    // sftp_close

    // FILE *fff = common::_fopen(_filepath, "rb+");
    // fread(_buffer, sizeof(char), _filesize, fff);
    // fwrite(_buffer, sizeof(char), _filesize, fff);
    // fclose(fff);

    std::string serverfile = field["name"];
    std::string ff = common::replace(serverfile, _source_path, "");
    std::string localfile = _destination_path + DIRSEPARATOR + ff;
    #ifdef _WIN32
    localfile = common::replace(localfile, "/", "\\");
    localfile = common::replace(localfile, "\\\\", "\\");
    #endif
    // vdlog::getInstance()->info("localfile: %s", localfile.c_str());

    if (field["type"] == YXU_S_IFDIR_CHAR)
    {
      common::_mkdir(localfile);
    }
    else
    {
      int access_type;
      sftp_file file;
      char buffer[MAX_XFER_BUF_SIZE];
      int nbytes, nwritten, rc, fc;

      // 打开服务器文件
      // 只读方式打开
      file = sftp_open(sftp, serverfile.c_str(), O_RDONLY, 0);
      if (file == NULL)
      {
        vdlog::getInstance()->error("Can't open file for reading: %s\n", ssh_get_error(this->conn->session));
        int ret1 = this->geterror();
        sftp_close(file);
        ptr_numbers(-1, -1, ret1);
        return;
      }

      // 打开本地文件
      // 只写打开二进制文件，不存在则创建，存在则清零
      FILE *fd;
      fd = common::_fopen(localfile, "wb+");
      if (fd==NULL)
      {
        vdlog::getInstance()->error("Can't open file for writing: %s", strerror(errno));
        vdlog::getInstance()->error("file: %s", localfile.c_str());
        vdlog::getInstance()->error("ssh_get_error: %s", ssh_get_error(this->conn->session));
        sftp_close(file);
        ptr_numbers(-1, -1, YXU_OPERATION_LOCAL_FILE_FAILED);
        return;
      }
      
      // ptr_numbers(filenumber, this->localfiles.size(), YXU_EMPTY);

      // 读取服务器文件的数据 ，向本地文件写入
      int wsize = 0;
      uint64_t _now = 0;
      for (;;)
      {

        this->transferlock->lock();
        if (*this->stoptransfer == 1)
        {
          this->transferlock->unlock();
          sftp_close(file);
          ptr_numbers(-1, -1, YXU_OPERATION_FAILED);
          return;
        }
        else
        {
          this->transferlock->unlock();
        }
sftp_async_read_label:
        int read_id = sftp_async_read_begin(file, sizeof(buffer));
        nbytes = sftp_async_read(file, buffer, sizeof(buffer), read_id);
        vdlog::getInstance()->error("nbytes: %d\n", nbytes);
        if (nbytes == 0)
        {
          // 回调函数
          ptr_progress(ff, wsize, std::stoi(field["size"]));
          break; // EOF
        }
        else if(nbytes==SSH_ERROR){
          vdlog::getInstance()->error("Error while reading file: %d, size: %d", sftp_get_error(this->sftp), nbytes);
          vdlog::getInstance()->error("Error allocating SFTP session: %s", ssh_get_error(this->conn->session));
          sftp_close(file);
          fc = fclose(fd);
          ptr_numbers(-1, -1, YXU_OPERATION_SERVER_FILE_FAILED);
          return;
        }
        else if(nbytes==SSH_AGAIN  ){
          goto sftp_async_read_label;
        }
        nwritten = fwrite(buffer, sizeof(char), nbytes, fd);
        if (nwritten != nbytes)
        {
          vdlog::getInstance()->error("Error writing: %s\n", strerror(errno));
          sftp_close(file);
          fc = fclose(fd);
          ptr_numbers(-1, -1, YXU_OPERATION_LOCAL_FILE_FAILED);
          return;
        }
        
        wsize = wsize + nbytes;
        // 防止回调函数调用过于频繁，导致主线程阻塞，这里每隔100毫秒调用一次
	      uint64_t timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()); // 获取时间戳（毫秒）
        // vdlog::getInstance()->warn("time: %lld", timestamp);
        if(timestamp>(_now+100)){
          ptr_progress(ff, wsize, std::stoi(field["size"]));
          _now = timestamp; 
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }

      // 关闭服务器打开的文件
      rc = sftp_close(file);
      // 关闭本地文件
      fc = fclose(fd);
    }
    // 回调函数
    filenumber = filenumber + 1;
    ptr_numbers(filenumber, this->serverfiles.size(), YXU_EMPTY);
  }
}

/**
 * @brief  遍历服务器目录 ，准备下载
 *
 * @param _dir 要递归的目录
 * @param mode 1 外部调用 ，2 递归函数调用
 * @return int
 */
int ysftp::downloadfiles_recursion_folder_server(std::string _dir, int mode)
{
  /// 判断目标路径是否存在，并且有读写权限
  sftp_attributes f_attr=NULL;
  int rc1 = this->getpermission(*this->conn->uid, _dir, f_attr);
  if(rc1!=YXU_SUCCESS){
    return rc1;
  }

  if ( (f_attr->permissions & SSH_S_IFDIR) == SSH_S_IFDIR)
  {
    map1 field;
    field["name"] = _dir;
    field["type"] = YXU_S_IFDIR_CHAR;
    this->serverfiles.push_back(field);
    //--
    std::string dirstr = _dir.substr(_dir.size() - 1, 1) == "/" ? _dir.substr(0, _dir.size() - 1) : _dir;
    sftp_dir dir = sftp_opendir(sftp, dirstr.c_str());
    if (dir == NULL)
    {
      return YXU_SFTP_ERROR;
    }
    sftp_attributes attr;
    while ((attr = sftp_readdir(sftp, dir)) != NULL)
    {
      if ((attr->permissions & SSH_S_IFDIR) == SSH_S_IFDIR)
      {
        if (std::string(attr->name) == "." || std::string(attr->name) == "..")
        {
          continue;
        }
        std::string childpath = dirstr + "/" + attr->name;
        this->downloadfiles_recursion_folder_server(childpath, 2);
      }
      else if ((attr->permissions & SSH_S_IFREG) == SSH_S_IFREG)
      {
        std::string filename = dirstr + "/" + std::string(attr->name);
        // vdlog::getInstance()->info("%s", filename.c_str());
        map1 field;
        field["name"] = filename;
        field["type"] = YXU_S_IFREG_CHAR;
        field["size"] = std::to_string(attr->size);
        this->serverfiles.push_back(field);
      }
      else
      {
        vdlog::getInstance()->error("file type error: %s...", attr->name);
      }
    }
  }
  else if ((f_attr->permissions & SSH_S_IFREG) == SSH_S_IFREG)
  {
    map1 field;
    field["name"] = _dir;
    field["type"] = YXU_S_IFREG_CHAR;
    field["size"] = std::to_string(f_attr->size);
    this->serverfiles.push_back(field);
  }
  else if ((f_attr->permissions & SSH_S_IFLNK) == SSH_S_IFLNK)
  {
    /// 忽略链接指向的目录，以普通文件的形式复制
    map1 field;
    field["name"] = _dir;
    field["type"] = YXU_S_IFLNK_CHAR;
    field["size"] = std::to_string(f_attr->size);
    this->serverfiles.push_back(field);
  }
  else
  {
    vdlog::getInstance()->warn("file type error: %s.", f_attr->name);
  }
  return YXU_SUCCESS;
}



int ysftp::downloadfiles_recursion_folder_server2(std::string _dir, int mode)
{
  sftp_attributes f_attr=NULL;
  ysftp_file_attr1 file_attr;
  int rc = this->get_filetype_and_permission(_dir, *this->conn->uid, f_attr, file_attr);
  if(rc!=YXU_SUCCESS){
    return rc;
  }else{
    if(file_attr.isRead==0){
      return YXU_FX_PERMISSION_DENIED;
    }
  }

  map1 field;
  if(file_attr.islink==1){
    if(file_attr.filetype=="dir"){
      /// 忽略链接指向的目录，以普通文件的形式复制
      field["name"] = file_attr.filepath;
      field["type"] = YXU_S_IFLNK_CHAR;
      field["size"] = std::to_string(f_attr->size);
      this->serverfiles.push_back(field);
    }
    else if(file_attr.filetype=="file"){
      field["name"] = file_attr.linkfilepath;
      field["type"] = YXU_S_IFREG_CHAR;
      field["size"] = std::to_string(f_attr->size);
      this->serverfiles.push_back(field);
    }
  }else{
    if(file_attr.filetype=="dir"){
      field["name"] = file_attr.filepath;
      field["type"] = YXU_S_IFDIR_CHAR;
      field["size"] = std::to_string(f_attr->size);
      this->serverfiles.push_back(field);
      ///
      std::string dirstr = _dir.substr(_dir.size() - 1, 1) == "/" ? _dir.substr(0, _dir.size() - 1) : _dir;
      sftp_dir dir = sftp_opendir(sftp, dirstr.c_str());
      if (dir == NULL)
      {
        return YXU_SFTP_ERROR;
      }
      sftp_attributes attr;
      while ((attr = sftp_readdir(sftp, dir)) != NULL)
      {
        if ((attr->permissions & SSH_S_IFDIR) == SSH_S_IFDIR)
        {
          if (std::string(attr->name) == "." || std::string(attr->name) == "..")
          {
            continue;
          }
          std::string childpath = dirstr + "/" + attr->name;
          this->downloadfiles_recursion_folder_server2(childpath, 2);
        }
        else if ((attr->permissions & SSH_S_IFREG) == SSH_S_IFREG)
        {
          std::string filename = dirstr + "/" + std::string(attr->name);
          field["name"] = filename;
          field["type"] = YXU_S_IFREG_CHAR;
          field["size"] = std::to_string(attr->size);
          this->serverfiles.push_back(field);
        }
        else
        {
          vdlog::getInstance()->error("file type error: %s", attr->name);
        }
      }
    }
    else if(file_attr.filetype=="file"){
      field["name"] = file_attr.filepath;
      field["type"] = YXU_S_IFREG_CHAR;
      field["size"] = std::to_string(f_attr->size);
      this->serverfiles.push_back(field);
    }
  }
  return 0;
}

int ysftp::exists_dir(std::string _path)
{
  /// 判断目标路径是否存在，并且有读写权限
  sftp_attributes _attr = NULL;
  int rc = this->getpermission(*this->conn->uid, _path, _attr);
  if(rc!=YXU_SUCCESS){
    return rc;
  }
}

int ysftp::_mkdir(std::string _path)
{
  int rc = sftp_mkdir(sftp, _path.c_str(), 0777);
  if (rc == SSH_OK)
  {
    return YXU_SUCCESS;
  }
  else
  {
    vdlog::getInstance()->error("_mkdir error: %s\n", ssh_get_error(this->conn->session));
    return this->geterror();
  }
}

int ysftp::_rmdir(std::string _path)
{
  int rc = sftp_rmdir(sftp, _path.c_str());
  if (rc == SSH_OK)
  {
    return YXU_SUCCESS;
  }
  else
  {
    vdlog::getInstance()->error("_rmdir error: %s\n", ssh_get_error(this->conn->session));
    return this->geterror();
  }
}

int ysftp::_rmfile(std::string _path)
{
  int rc = sftp_unlink(sftp, _path.c_str());
  if (rc == SSH_OK)
  {
    return YXU_SUCCESS;
  }
  else
  {
    vdlog::getInstance()->error("_rmfile error: %s\n", ssh_get_error(this->conn->session));
    return this->geterror();
  }
}

int ysftp::_rename(std::string _ori, std::string _new)
{
  int rc = sftp_rename(sftp, _ori.c_str(), _new.c_str());
  if (rc == SSH_OK)
  {
    return YXU_SUCCESS;
  }
  else
  {
    vdlog::getInstance()->error("_rename error: %s\n", ssh_get_error(this->conn->session));
    return this->geterror();
  }
}

int ysftp::_readfilecheck(std::string _path)
{
  sftp_attributes attr=NULL;
  ysftp_file_attr1 f_attr;
  int ret = this->get_filetype_and_permission(_path, *this->conn->uid, attr, f_attr);
  if (ret != YXU_SUCCESS)
  {
    return ret;
  }
  if(f_attr.isRead==0){
    return YXU_FX_PERMISSION_DENIED;
  }
  if(f_attr.isWrite==0){
    return YXU_NO_WRITE_PERMISSION;
  }
  const int READ_MAX_BYTES = 8 * 1024 * 1024; // 8MB
  if (attr->size > READ_MAX_BYTES)
  {
    return YXU_FILE_TOO_LARGE;
  }
  return YXU_SUCCESS;
}

int ysftp::_readPermission(std::string _path)
{
  sftp_attributes attr=NULL;
  ysftp_file_attr1 f_attr;
  int ret = this->get_filetype_and_permission(_path, *this->conn->uid, attr, f_attr);
  if (ret != YXU_SUCCESS)
  {
    return ret;
  }
  if(f_attr.isRead==0){
    return YXU_FX_PERMISSION_DENIED;
  }
  return YXU_SUCCESS;
}

int ysftp::_readfile(std::string _path, std::string &_content)
{
  
  sftp_attributes attr=NULL;
  ysftp_file_attr1 f_attr;
  int ret = this->get_filetype_and_permission(_path, *this->conn->uid, attr, f_attr);
  if (ret != YXU_SUCCESS)
  {
    return ret;
  }
  if(f_attr.isRead==0){
    return YXU_FX_PERMISSION_DENIED;
  }
  // if(f_attr.isWrite==0){
  //   return YXU_NO_WRITE_PERMISSION;
  // }

  int rc;
  // ToDo: 开始读取
  sftp_file file = sftp_open(sftp, _path.c_str(), O_RDONLY, 0);
  if (file == NULL)
  {
    return YXU_READ_FILE_FAILED;
  }
  const int READ_MAX_BYTES = 8 * 1024 * 1024; // 8MB
  if (attr->size > READ_MAX_BYTES)
  {
    return YXU_FILE_TOO_LARGE;
  }

  char buffer[MAX_XFER_BUF_SIZE];
  int nbytes;
  for (;;)
  {
    nbytes = sftp_read(file, buffer, sizeof(buffer));
    if (nbytes == 0)
    {
      break;
    }
    else if (nbytes < 0)
    {
      sftp_close(file);
      return YXU_READ_FILE_FAILED;
    }
    _content.append(buffer, nbytes);
  }
  rc = sftp_close(file);
  if (rc != SSH_OK)
  {
    return YXU_READ_FILE_FAILED;
  }
  return YXU_SUCCESS;
}

int ysftp::_writefile(std::string _path, std::string &_content)
{
  // 打开服务器文件
  sftp_file file = sftp_open(sftp, _path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
  if (file == NULL)
  {
    int err = this->geterror();
    vdlog::getInstance()->error("(sftp_open)Error: Code: %d", err);
    return err;
  }
  int length = _content.size();
  int nwritten;
  nwritten = sftp_write(file, _content.data(), _content.size());
  if (nwritten != length)
  {
    return YXU_WRITE_FILE_FAILED;
  }
  int rc = sftp_close(file);
  if (rc != SSH_OK)
  {
    return YXU_WRITE_FILE_FAILED;
  }
  return YXU_SUCCESS;
}

int ysftp::geterror()
{
  int err=sftp_get_error(sftp);
  if (err == SSH_FX_EOF)
  {
    // End-of-file encountered，遇到文件结尾
    return YXU_FX_EOF;
  }
  else if (err == SSH_FX_NO_SUCH_FILE)
  {
    // File doesn't exist  文件不存在
    return YXU_FX_NO_SUCH_FILE;
  }
  else if (err == SSH_FX_PERMISSION_DENIED)
  {
    // Permission denied  权限拒绝
    return YXU_FX_PERMISSION_DENIED;
  }
  else if (err == SSH_FX_FAILURE)
  {
    // Generic failure， 通用错误
    return YXU_FX_FAILURE;
  }
  else if (err == SSH_FX_BAD_MESSAGE)
  {
    // Garbage received from server， 从服务器接收到垃圾数据
    return YXU_FX_BAD_MESSAGE;
  }
  else if (err == SSH_FX_NO_CONNECTION)
  {
    // No connection has been set up , 没有建立连接
    return YXU_FX_NO_CONNECTION;
  }
  else if (err == SSH_FX_CONNECTION_LOST)
  {
    // There was a connection, but we lost it， 曾是一个连接，但我们丢失了它
    return YXU_FX_CONNECTION_LOST;
  }
  else if (err == SSH_FX_OP_UNSUPPORTED)
  {
    // Operation not supported by the server， 服务器不支持当前操作
    return YXU_FX_OP_UNSUPPORTED;
  }
  else if (err == SSH_FX_INVALID_HANDLE)
  {
    // Invalid file handle  无效的文件句柄
    return YXU_FX_INVALID_HANDLE;
  }
  else if (err == SSH_FX_NO_SUCH_PATH)
  {
    // No such file or directory path exists  没有搜索到文件或目录
    return YXU_FX_NO_SUCH_PATH;
  }
  else if (err == SSH_FX_FILE_ALREADY_EXISTS)
  {
    // An attempt to create an already existing file or directory has been made， 试图创建一个已经存在的文件或目录
    return YXU_FX_FILE_ALREADY_EXISTS;
  }
  else if (err == SSH_FX_WRITE_PROTECT)
  {
    // We are trying to write on a write-protected filesystem，尝试写一个写保护文件系统
    return YXU_FX_WRITE_PROTECT;
  }
  else if (err == SSH_FX_NO_MEDIA)
  {
    // No media in remote drive，远程驱动器中没有媒体
    return YXU_FX_NO_MEDIA;
  }
  else
  {
    // Unknown error  未知错误
    return YXU_OPERATION_FAILED;
  }
}

int ysftp::getpermission(std::string uid, std::string file, sftp_attributes &_attr)
{
  _attr = sftp_stat(sftp, file.c_str());
  if(_attr==NULL){
    auto _err = sftp_get_error(sftp);
    if (_err == SSH_FX_NO_SUCH_FILE) {
      // printf("文件不存在\n");
      return YXU_FX_NO_SUCH_PATH;
    }
    else if(_err = SSH_FX_PERMISSION_DENIED){
      // printf("没有权限\n");
      return YXU_FX_PERMISSION_DENIED;
    }
    else {
      // printf("无法获取文件属性\n");
      return this->geterror();
    }
  }else{
    // 判断权限
    if(_attr->uid==std::stoi(uid)){
      // 属于登录用户
      if ((_attr->permissions & 0400) && (_attr->permissions & 0200)) {
        // printf("文件具有所有者读写权限\n");
        return YXU_SUCCESS;
      } else {
        // printf("文件不具有所有者读写权限\n");
        return YXU_FX_PERMISSION_DENIED;
      }
    }else{
      // 不属于登录用户
      if(std::stoi(uid)==0){
        return YXU_SUCCESS;
      }
      else{
        if ((_attr->permissions & 0004)) {
          // printf("有读权限\n");
          return YXU_SUCCESS;
        } else {
          // printf("不具有读权限\n");
          return YXU_FX_PERMISSION_DENIED;
        }
      }
    }
  }
}

/**
 * @brief 获取文件的类型、以及读写权限
 * 
 * @param file  文件地址
 * @param uid       登录用户的ID
 * @param _attr     保存sftp_attributes
 * @param f_attr    保存返回的结果
 * @return int 
 */
int ysftp::get_filetype_and_permission(std::string file, std::string uid, sftp_attributes &_attr, ysftp_file_attr1 &f_attr)
{
  
  f_attr.filepath = file;
  f_attr.islink = 0;
  f_attr.isRead = 1;

  _attr = sftp_stat(sftp, file.c_str());
  if(_attr==NULL){
    auto _err = sftp_get_error(sftp);
    if (_err == SSH_FX_NO_SUCH_FILE) {
      // printf("文件不存在\n");
      return YXU_FX_NO_SUCH_PATH;
    }
    else if(_err = SSH_FX_PERMISSION_DENIED){
      // printf("没有权限\n");
      return YXU_FX_PERMISSION_DENIED;
    }
    else {
      // printf("无法获取文件属性\n");
      return this->geterror();
    }
  }
  
  // 如果文件是一个链接 ，需要读取链接指向的源文件
  if ((_attr->permissions & SSH_S_IFLNK) == SSH_S_IFLNK)
  {
    f_attr.islink = 1;
    char *_buffer = sftp_readlink(sftp, f_attr.filepath.c_str());
    if (_buffer == NULL)
    {
      vdlog::getInstance()->error("Invalid link: %s", _attr->name);
      f_attr.islink = -1;
    }
    else
    {
      ssh_string_free_char(_buffer);
      std::string linkfile(_buffer);
      if (linkfile.find_first_of("/") == 0)
      {
        f_attr.linkfilepath = linkfile;
      }
      else
      {
        std::string path = common::replace(file,  _attr->name, "");
        f_attr.linkfilepath = path + "/" + linkfile;
      }
      sftp_attributes l_attr = sftp_stat(sftp, f_attr.linkfilepath.c_str());
      if (l_attr == NULL)
      {
        // Invalid link
        vdlog::getInstance()->error("Invalid link: %s", linkfile.c_str());
        f_attr.islink = -1;
      }
      else
      {
        if ((l_attr->permissions & SSH_S_IFREG) == SSH_S_IFREG)
        {
          f_attr.filetype = "file";
        }
        if ((l_attr->permissions & SSH_S_IFDIR) == SSH_S_IFDIR)
        {
          f_attr.filetype = "dir";
        }
        // 判断链接源文件的权限
        if(l_attr->uid==std::stoi(uid)){
          // 属于登录用户
          if ((l_attr->permissions & 0400)) {
            // printf("所有者有读权限\n");
            f_attr.isRead = 1;
          } else {
            // printf("所有者无读权限\n");
            f_attr.isRead = 0;
          }
          if ((l_attr->permissions & 0200)) {
            // printf("所有者有写权限\n");
            f_attr.isWrite = 1;
          } else {
            // printf("所有者无写权限\n");
            f_attr.isWrite = 0;
          }
        }else{
          // 不属于登录用户
          if(std::stoi(uid)==0){
            f_attr.isWrite = 1;
            f_attr.isRead = 1;
          }
          else{
            if ((l_attr->permissions & 0004)) {
              // printf("有读权限\n");
              f_attr.isRead = 1;
            } else {
              // printf("无读权限\n");
              f_attr.isRead = 0;
            }
            //
            if ((l_attr->permissions & 0002)) {
              // printf("有写权限\n");
              f_attr.isWrite = 1;
            } else {
              // printf("无写权限\n");
              f_attr.isWrite = 0;
            }
          }
        }
      }
    }
  }
  else
  {
    if ((_attr->permissions & SSH_S_IFREG) == SSH_S_IFREG)
    {
      f_attr.filetype = "file";
    }
    if ((_attr->permissions & SSH_S_IFDIR) == SSH_S_IFDIR)
    {
      f_attr.filetype = "dir";
    }
    // 判断权限
    if(_attr->uid==std::stoi(uid)){
      // 属于登录用户
      if ((_attr->permissions & 0400)) {
        // printf("所有者有读权限\n");
        f_attr.isRead = 1;
      } else {
        // printf("所有者无读权限\n");
        f_attr.isRead = 0;
      }
      if ((_attr->permissions & 0200)) {
        // printf("所有者有写权限\n");
        f_attr.isWrite = 1;
      } else {
        // printf("所有者无写权限\n");
        f_attr.isWrite = 0;
      }
    }else{    
      // 不属于登录用户
      if(std::stoi(uid)==0){
        f_attr.isWrite = 1;
        f_attr.isRead = 1;
      }
      else{
        if ((_attr->permissions & 0004)) {
          // printf("有读权限\n");
          f_attr.isRead = 1;
        } else {
          // printf("无读权限\n");
          f_attr.isRead = 0;
        }
        //
        if ((_attr->permissions & 0002)) {
          // printf("有写权限\n");
          f_attr.isWrite = 1;
        } else {
          // printf("无写权限\n");
          f_attr.isWrite = 0;
        }
      }
    }
  }
  return YXU_SUCCESS;
}
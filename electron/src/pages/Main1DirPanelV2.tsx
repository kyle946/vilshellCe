import * as React from 'react'
import i18n from 'i18next';
import { Trans, Translation, withTranslation } from 'react-i18next'
import { type PropsWithChildren, useState, useRef, useEffect } from 'react'
import * as ReactDOM from 'react-dom'
import { createHashHistory, LocationDescriptorObject, History } from "history";
const history = createHashHistory();
import fs from "fs"
import path from "path"

import { Tabs, Spin, TabPane, Button, Progress } from '@douyinfe/semi-ui';
import * as Common from "../components/Common";
import { yxulinux_connect_state, yxulinux_file_type as ft, yxulinux_error_code as cc } from "../types/ErrorCode";
import * as dlg from "../components/Dialog";
import { confirmdlg, rm_confirmdlg } from "../components/ConfirmDialog";
import { PopupMenudlg, type PopupMenuItem } from "../components/PopupMenu"

import "./Main1file.less"

type FileTransferProgressV2Props = PropsWithChildren<{
  color: string,
  filename?: string,   // 正在传输的文件名
  size?: number,       // 正在传输字节大小
  count?: number,      // 正在传输的文件大小
  num?: number,        // 已传文件数量
  nums?: number,       // 文件总数
}>;
const FileTransferProgressV2: React.FC<FileTransferProgressV2Props> = (props: FileTransferProgressV2Props) => {

  let _filename: string = props.filename || "...";   // 正在传输的文件名
  let _size: number = props.size || 0;       // 正在传输字节大小
  let _count: number = props.count || 0;      // 正在传输的文件大小
  let _num: number = props.num || 0;        // 已传文件数量
  let _nums: number = props.nums || 0;       // 文件总数

  let percentage1 = Math.round((_num / _nums) * 100);
  let percentage2 = Math.round((_size / _count) * 100);

  return (
    <div className='FileTransferProgress1'>
      <>
        <div className='progress'>
          <div className='t'><span>{_num} / {_nums}</span><span>{percentage1 || 0}%</span></div>
          <div className='b'>
            <Progress percent={percentage1 || 0} stroke={[{ percent: 0, color: props.color }, { percent: 100, color: props.color }]} />
          </div>
        </div>

        <div className='progress'>
          <div className='tname'>
            <div className='name'>{_filename}</div>
          </div>
          <div className='t'><span>{Common.renderSize(_size)} / {Common.renderSize(_count)}</span><span>{percentage2 || 0}%</span></div>
          <div className='b'>
            <Progress percent={percentage2 || 0} stroke={[{ percent: 0, color: props.color }, { percent: 100, color: props.color }]} />
          </div>
        </div>
      </>
    </div>
  );
};



/**************************************************************************************************************************************************************************
 *=========================================================================================================================================================================
 *
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *=========================================================================================================================================================================
 **************************************************************************************************************************************************************************/

type Main1DirItemSerV2Type = {
  index: number,                      //文件在目录中的序号
  fileobj: any,
  select?: boolean,                   //文件是否被选中
  opendir: (_path: string) => void,     //鼠标双击，打开文件夹
  onclick?: () => void,      //鼠标单击，选中文件
  ondragend: (event: React.DragEvent) => void,
  ondragstart: (event: React.DragEvent) => void,   //拖拽文件
  oncontextmenu: (e: React.MouseEvent) => void,   //单击鼠标右键
  OnOpen?: (i: number) => void,         //右键菜单，点打开
  OnOpenNewTab?: (i: number) => void,   //在新标签打开
  OnOpenEditor?: (i: number) => void,   //在编辑器打开
  OnRename?: (i: number) => void,       //重命名
  OnDownload?: (i: number) => void,     //下载
  OnUpload?: (i: number) => void,       //上传
  OnDelete?: (i: number) => void,       //删除
}
type Main1DirItemSerV2Props = PropsWithChildren<Main1DirItemSerV2Type>;
const Main1DirItemSerV2: React.FC<Main1DirItemSerV2Props> = (props: Main1DirItemSerV2Props) => {

  const ondoubleclick = () => {
    if (props.fileobj.filetype == "file") {
      //
    }
    else if (props.fileobj.filetype == "dir") {
      if (props.fileobj.islink == 1) {
        props.opendir(props.fileobj.linkfilepath);
      }
      else if (props.fileobj.islink == -1) {
        dlg.alert(i18n.t('error'), i18n.t("Invalid folder."));
      }
      else if (props.fileobj.islink == 0) {
        props.opendir(props.fileobj.filepath);
      }
    }
  }

  const renderIcon = (t: string) => {
    if (t == "file") {
      if (props.fileobj.islink == 1) {
        return <><i className='iconfont iconfile blue'>&#xe614;</i><i className='iconfont iconlnk'>&#xe695;</i></>
      }
      else if (props.fileobj.islink == -1) {
        return <><i className='iconfont iconfile red'>&#xe614;</i><i className='iconfont iconlnk'>&#xe61a;</i></>
      }
      else if (props.fileobj.islink == 0) {
        return <i className='iconfont iconfile'>&#xe614;</i>
      }
    }
    else if (t == "dir") {
      if (props.fileobj.islink == 1) {
        return <><i className='iconfont icondir blue'>&#xe648;</i><i className='iconfont iconlnk'>&#xe695;</i></>
      }
      else if (props.fileobj.islink == -1) {
        return <><i className='iconfont icondir red'>&#xe648;</i><i className='iconfont iconlnk'>&#xe61a;</i></>
      }
      else if (props.fileobj.islink == 0) {
        return <i className='iconfont icondir blue'>&#xe648;</i>
      }
    }
  }

  let txtstr = props.fileobj.longname as string;
  let txtarr = txtstr.split(/[\s\t]+/);

  return (
    <div className='Main1FileItem1' onContextMenu={props.oncontextmenu}
      draggable="true"
      onDragEnd={props.ondragend}
      onDragStart={props.ondragstart} 
      >
      <a className={props.select ? "item1 active" : "item1"}
        title={props.fileobj.name}
        onClick={props.onclick}
        onDoubleClick={ondoubleclick}>
        <div className='icon1'>{renderIcon(props.fileobj.filetype)}</div>
        <div className='right1'>
          <div className='rt1'>{props.fileobj.name}</div>
          <div className='rb1'>{txtarr[0]} | {txtarr[2]} | {txtarr[3]}</div>
          <div className='rb2'>{Common.dateTranslate1(props.fileobj.mtime, 5)} | {Common.renderSize(props.fileobj.size)}</div>
        </div>
      </a>
    </div>
  )
};


/**************************************************************************************************************************************************************************
 *=========================================================================================================================================================================
 *
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *=========================================================================================================================================================================
 **************************************************************************************************************************************************************************/


type Main1DirItemLocalTypeV2 = {
  path: string,                       //目录路径
  fileobj: any,
  select?: boolean,                   //文件是否被选中
  opendir: (_path: string) => void,     //鼠标双击，打开文件夹
  onclick?: () => void,      //鼠标单击，选中文件
  ondragend: (event: React.DragEvent) => void,
  ondragstart: (event: React.DragEvent) => void,   //拖拽文件
  oncontextmenu: (e: React.MouseEvent) => void,   //单击鼠标右键
  OnOpen?: (i: number) => void,         //右键菜单，点打开
  OnRename?: (i: number) => void,       //重命名
  OnUpload?: (i: number) => void,       //上传
  OnDelete?: (i: number) => void,       //删除
}
type Main1DirItemLocalV2Props = PropsWithChildren<Main1DirItemLocalTypeV2>;
const Main1DirItemLocalV2: React.FC<Main1DirItemLocalV2Props> = (props: Main1DirItemLocalV2Props) => {
  const ondoubleclick = () => {
    if (props.fileobj.filetype == "file") {
      let _path = props.path + global.DIRSEPA + props.fileobj.name;
      //
    }
    else if (props.fileobj.filetype == "dir") {
      let _path = props.path + global.DIRSEPA + props.fileobj.name;
      props.opendir(_path);
    }
  }


  const renderIcon = (t: string) => {
    if (t == "file") {
      if (props.fileobj.islink == 1) {
        return <><i className='iconfont iconfile '>&#xe614;</i><i className='iconfont iconlnk'>&#xe695;</i></>
      }
      else if (props.fileobj.islink == -1) {
        return <><i className='iconfont iconfile red'>&#xe614;</i><i className='iconfont iconlnk'>&#xe61a;</i></>
      }
      else if (props.fileobj.islink == 0) {
        return <i className='iconfont iconfile'>&#xe614;</i>
      }
    }
    else if (t == "dir") {
      if (props.fileobj.islink == 1) {
        return <><i className='iconfont icondir orange'>&#xe648;</i><i className='iconfont iconlnk'>&#xe695;</i></>
      }
      else if (props.fileobj.islink == -1) {
        return <><i className='iconfont icondir red'>&#xe648;</i><i className='iconfont iconlnk'>&#xe61a;</i></>
      }
      else if (props.fileobj.islink == 0) {
        return <i className='iconfont icondir orange'>&#xe648;</i>
      }
    }
  }

  return (
    <div className='Main1FileItem1' onContextMenu={props.oncontextmenu}
      draggable="true"
      onDragEnd={props.ondragend}
      onDragStart={props.ondragstart}
    >
      <a className={props.select ? "item1 active" : "item1"}
        title={props.fileobj.name}
        onClick={props.onclick}
        onDoubleClick={ondoubleclick}>
        <div className='icon2'>{renderIcon(props.fileobj.filetype)}</div>
        <div className='right1'>
          <div className='rt1'>{props.fileobj.name}</div>
          <div className='rb2'>{Common.dateTranslate1(props.fileobj.mtime, 5)} | {Common.renderSize(props.fileobj.size)}</div>
        </div>
      </a>
    </div>
  )
};


/**************************************************************************************************************************************************************************
 *=========================================================================================================================================================================
 *
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *=========================================================================================================================================================================
 **************************************************************************************************************************************************************************/
type Main1DirPanelV2Props = PropsWithChildren<{
  path_server: string,
  path_local: string,
  filetype: string,
  itemKey: string,
  ChangeDir: (dir: string) => void,
}>;

class Main1DirPanelV2 extends React.Component<Main1DirPanelV2Props> {

  public SelectedFilesServer: any[] = [];
  public SelectedFilesLocal: any[] = [];

  public inputref_server: React.RefObject<HTMLInputElement>;
  public inputref_local: React.RefObject<HTMLInputElement>;

  public filearea1_ref: React.RefObject<HTMLDivElement>;
  public filearea2_ref: React.RefObject<HTMLDivElement>;

  public ctrlstate: number = 0;
  public FileTransferServerState: number;
  public FileTransferLocalState: number;

  public CurPathSer: string;
  public CurPathLocal: string;

  public onselectStateSer: number;
  public onselectStateLocal: number;
  public current_area: string;

  public _filename_ser: string;
  public _size_ser: number;
  public _count_ser: number;
  public _num_ser: number;
  public _nums_ser: number;

  public _filename_local: string;
  public _size_local: number;
  public _count_local: number;
  public _num_local: number;
  public _nums_local: number;

  public _server_uuid: string;
  public _local_uuid: string;

  public _history_server: any[];
  public _history_server_action: boolean;
  public _history_server_index: number;

  public _history_local: any[];
  public _history_local_action: boolean;
  public _history_local_index: number;
  
  public opendir_ser_t: NodeJS.Timeout;
  public ExpansionPanel = true;


  constructor(props: Main1DirPanelV2Props) {
    super(props);
    this.CurPathSer = this.props.path_server;
    this.CurPathLocal = this.props.path_local;
    this.inputref_server = React.createRef<HTMLInputElement>();
    this.inputref_local = React.createRef<HTMLInputElement>();
    this.filearea1_ref = React.createRef<HTMLDivElement>();
    this.filearea2_ref = React.createRef<HTMLDivElement>();
    this._server_uuid = apphandle.getuuid();
    this._local_uuid = apphandle.getuuid();
    this.onselectStateSer = 0;
    this.onselectStateLocal = 0;
    this.current_area = "";
    this._num_ser = 0;
    this._num_local = 0;

    this._history_server = [];
    this._history_server_action = false;
    this._history_server_index = 0;

    this._history_local = [];
    this._history_local_action = false;
    this._history_local_index = 0;

    this.opendir_ser_t = null;

    this.keydonw1=this.keydonw1.bind(this);
    this.keyup1=this.keyup1.bind(this);
    this.rember_current_directory=this.rember_current_directory.bind(this);
    this.clear_current_directory=this.clear_current_directory.bind(this);
    this.opendir_ser=this.opendir_ser.bind(this);
    this.opendir_ser2=this.opendir_ser2.bind(this);
    this.goback_server = this.goback_server.bind(this);
    this.opendir_local=this.opendir_local.bind(this);
    this.opendir_local2=this.opendir_local2.bind(this);
    this.goback_local = this.goback_local.bind(this);
    this.parent_dir=this.parent_dir.bind(this);
    this.reload_dir=this.reload_dir.bind(this);
    this.generate_folder=this.generate_folder.bind(this);
    this.newterm=this.newterm.bind(this);
    this.newtab=this.newtab.bind(this);
    this.onContextMenuServer=this.onContextMenuServer.bind(this);
    this.onContextMenuLocal=this.onContextMenuLocal.bind(this);
    this.delete_file=this.delete_file.bind(this);
    this.rename_file=this.rename_file.bind(this);
    this.oncontextmenu_ser=this.oncontextmenu_ser.bind(this);
    this.oncontextmenu_local=this.oncontextmenu_local.bind(this);
    this.downloadfile=this.downloadfile.bind(this);
    this.uploadfile=this.uploadfile.bind(this);
    this.stop_transfer=this.stop_transfer.bind(this);
    this.input_enter_server=this.input_enter_server.bind(this);
    this.input_enter_local=this.input_enter_local.bind(this);
    this.onselect_fromser=this.onselect_fromser.bind(this);
    this.onselect_fromlocal=this.onselect_fromlocal.bind(this);
    this.onclickpancel_fromser=this.onclickpancel_fromser.bind(this);
    this.onclickpancel_fromlocal=this.onclickpancel_fromlocal.bind(this);
    this.onmouseuppancel_fromser=this.onmouseuppancel_fromser.bind(this);
    this.onmouseuppancel_fromlocal=this.onmouseuppancel_fromlocal.bind(this);

    this.ondrop_server=this.ondrop_server.bind(this);
    this.ondrop_local=this.ondrop_local.bind(this);
    this.dragstart_item_server = this.dragstart_item_server.bind(this);
    this.dragstart_item_local = this.dragstart_item_local.bind(this);

    this.state = {
      rand: 0.1
    }
  }

  keydonw1(e: any) {
    // console.log(e.key);
    if (e.ctrlKey) {
      this.ctrlstate = 1;
    }

    // 全选
    if (e.ctrlKey&&e.key=='a') {
      e.preventDefault();
      if(this.current_area=="server"){
        this.SelectedFilesServer = this.SelectedFilesServer.map((v: any, i: number) => {
          v.select = 1;
          return v;
        });
      }
      else if(this.current_area == "local"){
        this.SelectedFilesLocal = this.SelectedFilesLocal.map((v: any, i: number) => {
          v.select = 1;
          return v;
        });
      }
      this.setState({ rand: Math.random() });
    }

    // 返回上级
    if(e.key=='Backspace'){
      e.preventDefault();
      if(this.current_area=="server"){
        this.goback_server();
      }
      else if(this.current_area == "local"){
        this.goback_local();
      }
    }

    // 删除
    if(e.key=='Delete'){
      if(this.current_area=="server"){
        //判断是否选中了多个文件
        let sel1: any[] = [];
        this.SelectedFilesServer.forEach((v: any, i: number, t: []) => {
          if (1 == v.select) {
            sel1.push(v);
          }
        });
        if(sel1.length>0){
          this.delete_file(sel1, "server");
        }
      }
      else if(this.current_area == "local"){
        //判断是否选中了多个文件
        let sel1: any[] = [];
        this.SelectedFilesLocal.forEach((v: any, i: number, t: []) => {
          if (1 == v.select) {
            sel1.push(v);
          }
        });
        if(sel1.length>0){
          this.delete_file(sel1, "local");
        }
      }
    }
    // console.log(e);
  }

  keyup1(e: any) {
    if (e.key=='Control') {
      this.ctrlstate = 0;
    }
  }

  componentDidMount(): void {
    this.opendir_ser(this.CurPathSer);
    this.opendir_local(this.CurPathLocal);

    if(this.filearea1_ref.current){
      this.filearea1_ref.current.setAttribute('tabindex', '0');
      this.filearea1_ref.current.addEventListener("keydown", this.keydonw1);
      this.filearea1_ref.current.addEventListener("keyup", this.keyup1);
    }

    if(this.filearea2_ref.current){
      this.filearea2_ref.current.setAttribute('tabindex', '0');
      this.filearea2_ref.current.addEventListener("keydown", this.keydonw1);
      this.filearea2_ref.current.addEventListener("keyup", this.keyup1);
    }
  }

  componentWillUnmount(): void {
    this.filearea1_ref.current.removeEventListener("keydown", this.keydonw1);
    this.filearea1_ref.current.removeEventListener("keyup", this.keyup1);
    this.filearea2_ref.current.removeEventListener("keydown", this.keydonw1);
    this.filearea2_ref.current.removeEventListener("keyup", this.keyup1);
  }

  /**
   * 记住当前目录，下次自动打开
   */
  rember_current_directory() {
    
    let filename = global.vilssh.hostinfo.hostlink+".ini";
    let hostini = apphandle.appinfo.vilshelldir + global.DIRSEPA + filename;

    var confjsonstr = "";
    if (fs.existsSync(hostini)) {
      confjsonstr = apphandle.readfile(hostini);
    }
    let conf: Map<string, string>;
    if (confjsonstr == "") {
      conf = new Map();
      conf.set(this.inputref_server.current.value, this.inputref_local.current.value);
    } else {
      conf = new Map(JSON.parse(confjsonstr));
      if (conf.size > 4) {
        dlg.alert(i18n.t('tips'), "目录数量已经达到5个，如果需要继续添加，请先清除。");
        return;
      }
      conf.set(this.inputref_server.current.value, this.inputref_local.current.value);
    }
    let jsonText = JSON.stringify(Array.from(conf.entries()));
    apphandle.logerr(jsonText);
    try {
      apphandle.writefile(hostini, jsonText);
      dlg.alert(i18n.t('tips'), "已记住当前服务器目录和本地目录，下次连接主机将自动加载。");
    } catch (error) {
      apphandle.logerr(error);
    }
    
  }

  /**
   * 清理所有记住的目录
   */
  clear_current_directory() {
    //begin
    let oncancel = () => { };
    let onyes = () => { };
    let onconfirm = () => {
      
      let filename = global.vilssh.conf.user+'@'+global.vilssh.conf.host+".ini";
      let hostini = apphandle.appinfo.vilshelldir + global.DIRSEPA + filename;
      
      if (fs.existsSync(hostini)) {
        apphandle.write_file(hostini, "");
        dlg.alert(i18n.t('tips'), "清除成功。");
      }

    };
    confirmdlg(
      i18n.t('tips'),
      "你确定要清除记住的目录吗？",
      "Cancel", "", "Confirm",
      oncancel, onyes, onconfirm, 12);
    //end
  };

  // 双击打开目录：服务器
  opendir_ser(_path: string) {
    if (this.opendir_ser_t != null) {
      clearTimeout(this.opendir_ser_t);
      this.opendir_ser_t = null;
    }
    this.opendir_ser_t = setTimeout(() => {
      this.opendir_ser2(_path);
    }, 200);
  };
  opendir_ser2(_path: string) {
    let c_path = _path.replace(/\/\//g, "/");

    if(this._history_server_action){    // 是 GoBack 点击过来的，不要增加历史记录
      if(this._history_server_index>0){
        this._history_server = this._history_server.slice(0, this._history_server_index);
      }
    }else{
      this._history_server.push(c_path);
      this._history_server_index++;
    }

    this._history_server_action = false;
    global.ievt.emit("showload");
    global.vilssh.listdir(c_path).then((res: object[]) => {
      global.ievt.emit("closeload");
      this.inputref_server.current.value = c_path;
      this.CurPathSer = c_path;
      this.props.ChangeDir(path.basename(c_path));
      this.SelectedFilesServer = [];
      this.SelectedFilesServer = res;
      this.setState({ rand: Math.random() });
    }).catch((err: number) => {
      console.error(err);
      global.ievt.emit("closeload");
      dlg.alert(i18n.t('error'), i18n.t(`error_code_${err}`));
    });
  };

  goback_server(){
    if(this._history_server.length==0){
      return ;
    }
    this._history_server_action = true;
    this._history_server_index--;
    if(this._history_server_index<1){
      this._history_server_index=1;
    }
    let path = this._history_server[this._history_server_index - 1];
    this.opendir_ser(path);
  }


  // 双击打开目录： 本地
  public opendir_local_t: NodeJS.Timeout = null;
  opendir_local(_path: string) {
    if (this.opendir_local_t != null) {
      clearTimeout(this.opendir_local_t);
      this.opendir_local_t = null;
    }
    this.opendir_local_t = setTimeout(() => {
      this.opendir_local2(_path);
    }, 200);
  };
  opendir_local2(_path: string) {
    
    if(this._history_local_action){    // 是 GoBack 点击过来的，不要增加历史记录
      if(this._history_local_index>0){
        this._history_local = this._history_local.slice(0, this._history_local_index);
      }
    }else{
      this._history_local.push(_path);
      this._history_local_index++;
    }

    this.CurPathLocal = _path;
    this.inputref_local.current.value = this.CurPathLocal;
    this.SelectedFilesLocal = [];
    fs.readdir(this.CurPathLocal, (err, files) => {
      let filelist: any[] = [];
      let dirlist: any[] = [];
      if (!files) {
        return;
      }
      files.forEach((v, i, a) => {
        try {
          let _path1 = this.CurPathLocal + global.DIRSEPA + v;
          let filestat = fs.lstatSync(_path1);
          let filetype = "file";
          let size = filestat.size;
          let mtime = parseInt(filestat.mtimeMs / 1000 + '');
          let islink = 0;
          if (filestat.isSymbolicLink()) {
            islink = 1;
          }
          if (filestat.isDirectory()) {
            filetype = "dir";
            dirlist.push({ name: v, select: 0, filetype, size, mtime, islink });
          } else {
            filelist.push({ name: v, select: 0, filetype, size, mtime, islink });
          }
        } catch (err) {
          console.error(err);
        }
      });
      this.SelectedFilesLocal = dirlist.concat(filelist);
      this.setState({ rand: Math.random() });
    });
  };

  goback_local(){
    if(this._history_local.length==0){
      return ;
    }
    this._history_local_action = true;
    this._history_local_index--;
    if(this._history_local_index<1){
      this._history_local_index=1;
    }
    let path = this._history_local[this._history_local_index - 1];
    this.opendir_local(path);
  }

  // 返回上级目录
  parent_dir(type: string) {
    if (type == "server") {
      let _path = path.dirname(this.CurPathSer);
      this.opendir_ser(_path);
    }
    else if (type == "local") {
      let _path = path.dirname(this.CurPathLocal);
      this.opendir_local(_path);
    }
  };

  // 刷新目录
  reload_dir(type: string) {
    if (type == "server") {
      this.opendir_ser(this.CurPathSer);
    }
    else if (type == "local") {
      this.opendir_local(this.CurPathLocal);
    }
  };

  // 生成文件夹
  generate_folder(type: string) {
    if (type == "server") {
      dlg.confirminput("创建文件夹 [Server]", 4, "", (txt: string) => {
        if (txt == "") {
          return false;
        }
        let _path = this.CurPathSer + "/" + txt;
        global.vilssh._mkdir(_path).then(()=>{
          this.reload_dir('server');
        }).catch((err:any)=>{
          dlg.alert(i18n.t('error'), i18n.t(`error_code_${err}`));
        });
        return true;
      });
    }
    else if (type == "local") {
      dlg.confirminput("创建文件夹 [Local]", 4, "", (txt: string) => {
        if (txt == "") {
          return false;
        }
        let _path = this.CurPathLocal + "/" + txt;
        fs.mkdirSync(_path);
        this.reload_dir('local');
        return true;
      });
    }
  };

  /**
   * 在终端打开目录
   */
  newterm() {
    global.ievt.emit("newterm", this.CurPathSer);
    global.ievt.emit("onclickmenu", "term");
  }

  /**
   * 新标签打开
   */
  newtab() {
    global.ievt.emit("newtab", this.CurPathSer, this.CurPathLocal, "dir");
  };

  // 在目录空白处点鼠标右键: 服务器面板
  onContextMenuServer(e: React.MouseEvent) {
    const menu: PopupMenuItem[] = [
      { label: i18n.t("Go Back"), OnClick: () => this.goback_server() },
      { label: i18n.t("Parent Directory"), OnClick: () => this.parent_dir('server') },
      { label: i18n.t("Reload"), OnClick: () => this.reload_dir("server") },
      { label: i18n.t("New Folder"), OnClick: () => this.generate_folder("server") },
      { label: i18n.t("Open in Terminal"), OnClick: this.newterm },
      { label: i18n.t("Open in New Tab"), OnClick: this.newtab },
      { label: i18n.t("Remeber Directory"), OnClick: () => this.rember_current_directory() },
      { label: i18n.t("Clear Remeber Directory"), OnClick: () => this.clear_current_directory() }
    ]
    PopupMenudlg(e, menu);
  };

  // 在目录空白处点鼠标右键： 本地面板
  onContextMenuLocal(e: React.MouseEvent) {
    const menu: PopupMenuItem[] = [
      { label: i18n.t("Go Back"), OnClick: () => this.goback_local() },
      { label: i18n.t("Parent Directory"), OnClick: () => this.parent_dir('local') },
      { label: i18n.t("Reload"), OnClick: () => this.reload_dir("local") },
      { label: i18n.t("New Folder"), OnClick: () => this.generate_folder("local") },
    ]
    PopupMenudlg(e, menu);
  };

  // 删除文件
  delete_file(files: any[], type: string) {
    let oncancel = () => { };
    let onconfirm = () => {
      if (type == "server") {
        let filestr = "";
        files.forEach((v, i) => {
          filestr += `"${this.CurPathSer}/${v.name}" `
        });
        // 命令的方式删除文件和目录
        global.ievt.emit('showload');
        global.vilssh.execCmd(`rm ${filestr} -rf`)
        .then((res:string)=>{
          global.ievt.emit('closeload');
        }).catch((err:any)=>{
          global.ievt.emit('closeload');
          dlg.confirm(i18n.t('error'), i18n.t(`error_code_${err}`));
        });
      }
      else if (type == "local") {
        try {
          files.forEach((v, i) => {
            let _path = this.CurPathLocal + global.DIRSEPA + v.name;
            if (v.filetype == "file") {
              fs.unlinkSync(_path);
            }
            else if (v.filetype == "dir") {
              fs.rmdirSync(_path);
            }
          });
        } catch (error) {
          console.error(error);
        }
        this.reload_dir('local');
      }
      this.reload_dir(type);
      return true;
    };
    dlg.confirm(i18n.t('tips'), i18n.t('Are you sure you want to delete it?'), oncancel, onconfirm, 12);
  };

  // 重命名文件
  rename_file(_file: string, type: string) {
    let onconfirm = (txt: string) => {
      if (type == "server") {
        let _old = this.CurPathSer + '/' + _file;
        let _new = this.CurPathSer + '/' + txt;
        let ret = global.vilssh._rename(_old, _new);
        if (ret != cc.YXU_SUCCESS) {
          dlg.alert(i18n.t('error'), i18n.t(`error_code_${ret}`));
        } else {
          this.reload_dir('server');
        }
      }
      else if (type == "local") {
        let _old = this.CurPathLocal + global.DIRSEPA + _file;
        let _new = this.CurPathLocal + global.DIRSEPA + txt;
        fs.renameSync(_old, _new);
        this.reload_dir('local');
      }
      return true;
    };
    dlg.confirminput(`重命名文件[${_file}]`, 4, _file, onconfirm);
  };

  // 在文件上单击鼠标右键： 服务器面板
  oncontextmenu_ser(index: number, e: React.MouseEvent) {

    this.SelectedFilesServer = this.SelectedFilesServer.map((v: any, i: number) => {
      if (index == i) {
        v.select = 1;
      } else {
        // if (this.ctrlstate == 0) {
        //   v.select = 0;
        // }
      }
      return v;
    });
    this.onselectStateSer = 1;

    //判断是否选中了多个文件
    let sel1: any[] = [];
    this.SelectedFilesServer.forEach((v: any, i: number, t: []) => {
      if (1 == v.select) {
        sel1.push(v);
      }
    });
    this.setState({ rand: Math.random() });
    if (sel1.length == 1) {
      let ff = this.SelectedFilesServer[index];
      if (ff.filetype == "dir") {
        const menu: PopupMenuItem[] = [
          {
            label: i18n.t("Open"), OnClick: () => {
              let _path = this.CurPathSer + '/' + ff.name;
              this.opendir_ser(_path);
            }
          },
          {
            label: i18n.t("Open in New Tab"), OnClick: () => {
              let _path = this.CurPathSer + '/' + ff.name;
              global.ievt.emit("newtab", _path, "", ff.filetype);
            }
          },
          { label: i18n.t("Rename"), OnClick: () => this.rename_file(ff.name, "server") },
          { label: i18n.t("Download"), OnClick: () => this.downloadfile() },
          { label: i18n.t("Delete"), OnClick: () => this.delete_file(sel1, "server") },
        ]
        PopupMenudlg(e, menu);
      } else {
        const menu: PopupMenuItem[] = [
          { label: i18n.t("Rename"), OnClick: () => this.rename_file(ff.name, "server") },
          { label: i18n.t("Download"), OnClick: () => this.downloadfile() },
          { label: i18n.t("Delete"), OnClick: () => this.delete_file(sel1, "server") },
        ]
        PopupMenudlg(e, menu);
      }
    } else {
      const menu: PopupMenuItem[] = [
        { label: i18n.t("Download"), OnClick: () => this.downloadfile() },
        { label: i18n.t("Delete"), OnClick: () => this.delete_file(sel1, "server") },
      ]
      PopupMenudlg(e, menu);
    }
  }

  // 在文件上单击鼠标右键： 本地面板
  oncontextmenu_local(index: number, e: React.MouseEvent) {
    this.SelectedFilesLocal = this.SelectedFilesLocal.map((v: any, i: number) => {
      if (index == i) {
        v.select = 1;
      } else {
        // if (this.ctrlstate == 0) {
        //   v.select = 0;
        // }
      }
      return v;
    });
    this.onselectStateSer = 1;
    //判断是否选中了多个文件
    let sel1: any[] = [];
    this.SelectedFilesLocal.forEach((v: any, i: number, t: []) => {
      if (1 == v.select) {
        sel1.push(v);
      }
    });
    this.setState({ rand: Math.random() });
    if (sel1.length == 1) {
      let ff = this.SelectedFilesLocal[index];
      const menu: PopupMenuItem[] = [
        { label: i18n.t("Upload"), OnClick: () => this.uploadfile() },
        { label: i18n.t("Rename"), OnClick: () => this.rename_file(ff.name, "local") },
        { label: i18n.t("Delete"), OnClick: () => this.delete_file(sel1, "local") },
      ]
      PopupMenudlg(e, menu);
    } else {
      const menu: PopupMenuItem[] = [
        { label: i18n.t("Upload"), OnClick: () => this.uploadfile() },
        { label: i18n.t("Delete"), OnClick: () => this.delete_file(sel1, "local") },
      ]
      PopupMenudlg(e, menu);
    }
  };


  /**
   * 下载文件
   * @returns 
   */
  downloadfile() {
    ///
    if(this.FileTransferLocalState == 1){
      dlg.alert(i18n.t('tips'), i18n.t('Please wait for the file transfer to complete or stop the transfer manually.'));
      return;
    }
    ///
    let files: string[] = [];
    this.SelectedFilesServer.forEach((v: any, i: number, t: []) => {
      if (1 == v.select) {
        files.push(v.name);
      }
    });
    if (files.length == 0) {
      return;
    }
    let _call=(type:string)=>{
      // 开始传输
      this.FileTransferLocalState = 1;
      this.setState({ rand: Math.random() });
      let ptr_progress_call_js = (res: any) => {
        this._filename_local = res.name;
        this._size_local = res.size;
        this._count_local = res.count;
        this.setState({ rand: Math.random() });
      };
      let ptr_numbers_call_js = (res: any) => {
        this._num_local = res.num;
        this._nums_local = res.count;
        this.setState({ rand: Math.random() });
        if (res.num == res.count) {
          // 传输结束
          this.stop_transfer('local');
          if (res.num == -1) {
            // 传输错误
            dlg.alert(i18n.t('error'), i18n.t(`error_code_${res.code}`));
          } else {
            // 传输成功
            this.reload_dir('local');
          }
        }
        this.setState({ rand: Math.random() });
      };
      global.vilssh.downloadfiles(this._server_uuid, files, this.CurPathSer, this.CurPathLocal, ptr_progress_call_js, ptr_numbers_call_js);
      this.setState({ rand: Math.random() });
    };
    ///
    let oncancel=()=>{ };
    let onconfirm=()=>{
      _call('overwrite');
    };
    dlg.confirmbtntxt(
      i18n.t('tips'), 
      i18n.t('If you include a file with the same name, it will be overwritten by default. Do you want to continue?'), 
      i18n.t('Cancel'), 
      i18n.t('Continue'), 
      oncancel, onconfirm,
      12
    );
  };


  /**
   * 上传文件
   * @returns 
   */
  uploadfile() {
    if(this.FileTransferServerState == 1){
      dlg.alert(i18n.t('tips'), i18n.t('Please wait for the file transfer to complete or stop the transfer manually.'));
      return;
    }
    // 准备文件
    let files: string[] = [];
    this.SelectedFilesLocal.forEach((v: any, i: number, t: []) => {
      if (1 == v.select) {
        files.push(v.name);
      }
    });

    if (files.length == 0) {
      return;
    }
    let _call=(type:string)=>{

      // 开始传输
      this.FileTransferServerState = 1;
      this.setState({ rand: Math.random() });
      let ptr_progress_call_js = (res: any) => {
        this._filename_ser = res.name;
        this._size_ser = res.size;
        this._count_ser = res.count;
        this.setState({ rand: Math.random() });
      };
      let ptr_numbers_call_js = (res: any) => {
        this._num_ser = res.num;
        this._nums_ser = res.count;
        this.setState({ rand: Math.random() });
        if (res.num == res.count) {
          // 传输结束
          this.stop_transfer('server');
          if (res.num == -1) {
            // 传输错误
            dlg.alert(i18n.t('error'), i18n.t(`error_code_${res.code}`));
          } else {
            // 传输成功
            this.reload_dir('server');
          }
        }
      };
      global.vilssh.uploadfiles(this._local_uuid, files, this.CurPathLocal, this.CurPathSer, ptr_progress_call_js, ptr_numbers_call_js);
    };
    ///
    let oncancel=()=>{ };
    let onconfirm=()=>{
      _call('overwrite');
    };
    dlg.confirmbtntxt(
      i18n.t('tips'), 
      i18n.t('If you include a file with the same name, it will be overwritten by default. Do you want to continue?'), 
      i18n.t('Cancel'), 
      i18n.t('Continue'), 
      oncancel, onconfirm,
      12
    );
  };

  /**
   * 停止传输, 关闭传输进度窗口
   * @param type 
   */
  stop_transfer(type: string) {
    if (type == "server") {
      global.vilssh.stop_transfer(this.props.itemKey + 'ser');
      this._filename_ser = "";
      this._size_ser = 0;
      this._count_ser = 0;
      this._num_ser = 0;
      this._nums_ser = 0;
      this.FileTransferServerState = 0;
      this.setState({ rand: Math.random() });
    }
    else if (type == "local") {
      global.vilssh.stop_transfer(this.props.itemKey + 'local');
      this._filename_local = "";
      this._size_local = 0;
      this._count_local = 0;
      this._num_local = 0;
      this._nums_local = 0;
      this.FileTransferLocalState = 0;
      this.setState({ rand: Math.random() });
    }
  };

  /**
   * 在地址栏按回车
   */
  input_enter_server() {
    let _path = this.inputref_server.current.value;
    global.vilssh.get_filetype_and_permission(_path).then((robj:any)=>{
      let isread = false;
      if (robj.code== cc.YXU_SUCCESS) {
        if(robj.isRead==1){
          this.CurPathSer = _path;
          this.setState({ rand: Math.random() });
          this.opendir_ser(_path);
          isread=true;
        }
      }
      if(!isread){
        dlg.alert(i18n.t('error'), "目录不存在或者不可访问！");
      }
    }).catch((err:any)=>{
      global.ievt.emit('closeload');
      dlg.confirm(i18n.t('error'),i18n.t(`error_code_${err}`));
    });
  }

  /**
   * 在地址栏按回车
   */
  input_enter_local = () => {
    let _path = this.inputref_local.current.value;
    if(fs.existsSync(_path)){
      this.CurPathLocal = _path;
      this.setState({ rand: Math.random() });
      this.opendir_local(_path);
    }else{
      dlg.alert(i18n.t('error'), i18n.t("error_code_136"));
    }
  }

  /**
   * 当鼠标点击时，onselect，onclickpancel, onmouseuppancel 三个函数会同时执行，所以要区分先后顺序做判断
   * 1、onselect：当点击目录内的文件或项目，选中点击的文件，并onselectState=1，表示点击的是文件夹或文件
   * 2、onclickpancel：当点击目录时(包含了onselect)，如果鼠标点在某个文件或文件夹上时，什么也不会，如果点在了空白处，则取消所有选中的文件
   * 3、onmouseuppancel：当鼠标抬起时
   */

  /**
   * 当点击文件或文件夹时，选中点击的文件，并onselectState=1，表示点击的是文件夹或文件
   * @param index 
   */
  onselect_fromser(index: number) {
    // 
    let sel2 = this.SelectedFilesServer.map((v: any, i: number) => {
      if (index == i) {
        v.select == 0 ? v.select = 1 : v.select = 0;
      } else {
        if (this.ctrlstate == 0) {
          v.select = 0;
        }
      }
      return v;
    });
    this.SelectedFilesServer = sel2;
    this.onselectStateSer = 1;
    this.setState({ rand: Math.random() });
  };

  onselect_fromlocal(index: number) {
    // 
    let sel2 = this.SelectedFilesLocal.map((v: any, i: number) => {
      if (index == i) {
        v.select == 0 ? v.select = 1 : v.select = 0;
      } else {
        if (this.ctrlstate == 0) {
          v.select = 0;
        }
      }
      return v;
    });
    this.SelectedFilesLocal = sel2;
    this.onselectStateLocal = 1;
    this.setState({ rand: Math.random() });
  };

  /**
   * 当点击目录时(包含了onselect)，如果鼠标点在某个文件或文件夹上时，什么也不会，如果点在了空白处，则取消所有选中的文件
   * @param e 
   * @returns 
   */
  onclickpancel_fromser(e: React.MouseEvent) {
    this.current_area = "server";
    if (this.onselectStateSer == 1) {
      // 如果鼠标点在某个文件或文件夹上时，什么也不会
      return;
    } else {
      // 如果点在了空白处，则取消所有选中的文件
      let sel2 = this.SelectedFilesServer.map((v: any, i: number) => {
        v.select = 0;
        return v;
      });
      this.SelectedFilesServer = sel2;
    }
  };

  onclickpancel_fromlocal(e: React.MouseEvent) {
    this.current_area = "local";
    if (this.onselectStateLocal == 1) {
      // 如果鼠标点在某个文件或文件夹上时，什么也不会
      return;
    } else {
      // 如果点在了空白处，则取消所有选中的文件
      let sel2 = this.SelectedFilesLocal.map((v: any, i: number) => {
        v.select = 0;
        return v;
      });
      this.SelectedFilesLocal = sel2;
    }
  };

  /**
   * 当鼠标抬起时，还原到未点击状态
   * @param e 
   */
  onmouseuppancel_fromser(e: React.MouseEvent) {
    this.onselectStateSer = 0;
    this.setState({ rand: Math.random() });
  }

  onmouseuppancel_fromlocal(e: React.MouseEvent) {
    this.onselectStateLocal = 0;
    this.setState({ rand: Math.random() });
  }

  /**
   * 拖拽(上传)
   * @param event 
   */
  ondrop_server(event: React.DragEvent){
    event.preventDefault();
    const _files = event.dataTransfer.files;
    
    if(this.FileTransferServerState == 1){
      dlg.alert(i18n.t('tips'), i18n.t('Please wait for the file transfer to complete or stop the transfer manually.'));
      return;
    }
    ///
    let oncancel=()=>{ };
    let onconfirm=()=>{
      if(_files.length>0){
        let files:any=[];
        for(let i=0;i<_files.length;i++){
          files.push(_files.item(i).name);
        }
        let CurPathLocal = path.dirname(_files.item(0).path);
        // 开始传输
        this.FileTransferServerState = 1;
        // this.FileTransferLocalState = 1;
        this.setState({ rand: Math.random() });
        let ptr_progress_call_js = (res: any) => {
          this._filename_ser = res.name;
          this._size_ser = res.size;
          this._count_ser = res.count;
          //
          this._filename_local = res.name;
          this._size_local = res.size;
          this._count_local = res.count;
          //
          this.setState({ rand: Math.random() });
        };
        let ptr_numbers_call_js = (res: any) => {
          this._num_ser = res.num;
          this._nums_ser = res.count;
          //
          this._num_local = res.num;
          this._nums_local = res.count;
          //
          this.setState({ rand: Math.random() });
          if (res.num == res.count) {
            // 传输结束
            this.stop_transfer('server');
            // this.stop_transfer('local');
            if (res.num == -1) {
              // 传输错误
              dlg.alert(i18n.t('error'), i18n.t(`error_code_${res.code}`));
            } else {
              // 传输成功
              this.reload_dir('server');
            }
          }
        };
        global.vilssh.uploadfiles(this._local_uuid, files, CurPathLocal, this.CurPathSer, ptr_progress_call_js, ptr_numbers_call_js);
      }else{
        if(this.FileTransferServerState == 1){
          dlg.alert(i18n.t('tips'), i18n.t('Please wait for the file transfer to complete or stop the transfer manually.'));
          return;
        }
        // 准备文件
        let files: string[] = [];
        this.SelectedFilesLocal.forEach((v: any, i: number, t: []) => {
          if (1 == v.select) {
            files.push(v.name);
          }
        });
        if (files.length == 0) {
          return;
        }
        // 开始传输
        this.FileTransferServerState = 1;
        this.setState({ rand: Math.random() });
        let ptr_progress_call_js = (res: any) => {
          this._filename_ser = res.name;
          this._size_ser = res.size;
          this._count_ser = res.count;
          this.setState({ rand: Math.random() });
        };
        let ptr_numbers_call_js = (res: any) => {
          this._num_ser = res.num;
          this._nums_ser = res.count;
          this.setState({ rand: Math.random() });
          if (res.num == res.count) {
            // 传输结束
            this.stop_transfer('server');
            if (res.num == -1) {
              // 传输错误
              dlg.alert(i18n.t('error'), i18n.t(`error_code_${res.code}`));
            } else {
              // 传输成功
              this.reload_dir('server');
            }
          }
        };
        global.vilssh.uploadfiles(this._local_uuid, files, this.CurPathLocal, this.CurPathSer, ptr_progress_call_js, ptr_numbers_call_js);
      }
    };
    dlg.confirmbtntxt(
      i18n.t('tips'), 
      i18n.t('If you include a file with the same name, it will be overwritten by default. Do you want to continue?'), 
      i18n.t('Cancel'), 
      i18n.t('Continue'), 
      oncancel, onconfirm,
      12
    );
  }

  ondrop_local(event: React.DragEvent){
    event.preventDefault();
    // const files = event.dataTransfer.files;
    this.downloadfile();
  }

  // dragstart_item_server(v: any, event: React.DragEvent){
  //   event.dataTransfer.setData('DownloadURL',"image/png:yxulinux.png:data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAUAAAAGCAYAAAAL+1RLAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAAXSURBVBhXY/wPBAxogAlKowDqCzIwAADvkwQIleHGIAAAAABJRU5ErkJggg==");
  // }

  dragstart_item_server(i:number, event: React.DragEvent){
    console.log(i);
    this.SelectedFilesServer[i].select=1;
  }

  dragstart_item_local(i:number, event: React.DragEvent){
    console.log(i);
    this.SelectedFilesLocal[i].select=1;
  }

  render() {

    return (
      <div className='Main1FilePanel1'>

        {/* 服务器操作 */}
        <div className='top1'>
          <div className='icon1'><i className='iconfont blue'>&#xe648;</i></div>
          <div className='address1'>
            <span className='blue'>Server </span>
            <input className='grey1'
              defaultValue={this.CurPathSer}
              onKeyDown={
                (e: React.KeyboardEvent) => {
                  if (e.key == 'Enter') {
                    this.input_enter_server();
                  }
                }
              }
              ref={this.inputref_server} />
          </div>
          <div className='btnarea'>
            <a className='grey1' onClick={() => this.goback_server()}><i className='iconfont'>&#xe79a;</i>{i18n.t('GoBack')}</a>
            <a className='grey1' onClick={() => this.parent_dir('server')}><i className='iconfont i1'>&#xe79a;</i>{i18n.t('Parent')}</a>
            <a className='grey1' onClick={() => this.reload_dir('server')}><i className='iconfont'>&#xe61c;</i>{i18n.t('Reload')}</a>
            <a className='grey1' onClick={() => this.downloadfile()}><i className='iconfont'>&#xe74a;</i>{i18n.t('Down')}</a>
            <a className='grey1' onClick={() => this.generate_folder('server')}><i className='iconfont'>&#xe829;</i>{i18n.t('NewFolder')}</a>
            {/* <a className='grey1' onClick={(e) => this.close_transter_dialog('server')}>
              <i className='iconfont'>&#xe65d;</i>{i18n.t('Transfer')}({this._num_ser})</a> */}
          </div>
        </div>

        <div className={this.ExpansionPanel?'top2':'top2-2'}
          // draggable="true" 
          onDrop={this.ondrop_server} 
          onDragEnter={(e) => e.preventDefault()} 
          onDragOver={(e) => e.preventDefault()}
          onClick={(e) => setTimeout(() => this.onclickpancel_fromser(e), 50)}
          onMouseUp={(e) => setTimeout(() => this.onmouseuppancel_fromser(e), 60)}>
          <div className='filearea' onContextMenu={this.onContextMenuServer} ref={this.filearea1_ref}>
            {this.SelectedFilesServer.map((v: any, i: number) => {
              return <Main1DirItemSerV2
                fileobj={v}
                index={i}
                oncontextmenu={(e) => this.oncontextmenu_ser(i, e)}
                opendir={this.opendir_ser}
                onclick={() => setTimeout(() => this.onselect_fromser(i), 10)}
                select={v.select == 1 ? true : false}
                ondragend={(e) => {}}
                ondragstart={(e) =>this.dragstart_item_server(i, e)}
                key={i} />
            })}
            <div style={{ height: "50px", width: "100%" }}></div>
          </div>
          {this.FileTransferServerState == 1 && <>
            <div className='transferlist'>
              <div className='progress head'>
                <a onClick={() => this.stop_transfer('server')}><i className='iconfont'>&#xe61a;</i></a>
              </div>
              <FileTransferProgressV2
                color='#027bb3'
                filename={this._filename_ser}
                size={this._size_ser}
                count={this._count_ser}
                num={this._num_ser}
                nums={this._nums_ser}  />
            </div>
          </>}
        </div>

        {/* 本地操作 */}
        <div className='bottom1'>
          <div className='icon1'><i className='iconfont orange'>&#xe648;</i></div>
          <div className='address1'>
            <span className='grey1'>Local </span>
            <input className='grey1'
              defaultValue={this.CurPathLocal}
              onKeyDown={
                (e: React.KeyboardEvent) => {
                  if (e.key == 'Enter') {
                    this.input_enter_local();
                  }
                }
              }
              ref={this.inputref_local} />
          </div>
          <div className='btnarea'>
             <a className='grey1' onClick={() => this.goback_local() }><i className='iconfont'>&#xe79a;</i>{i18n.t('GoBack')}</a>
             <a className='grey1' onClick={() => this.parent_dir('local')}><i className='iconfont i1'>&#xe79a;</i>{i18n.t('Parent')}</a>
             <a className='grey1' onClick={() => this.reload_dir('local')}><i className='iconfont'>&#xe61c;</i>{i18n.t('Reload')}</a>
             {/* <a className='grey1' onClick={() => this.uploadfile()}><i className='iconfont'>&#xe74a;</i>{i18n.t('Upload')}</a> */}
            {/* <a className='grey1' onClick={() => this.generate_folder('local')}><i className='iconfont'>&#xe829;</i>{i18n.t('NewFolder')}</a> */}
            <a className='grey1' onClick={() => {
              this.ExpansionPanel = this.ExpansionPanel?false:true;
              this.setState({rand:Math.random()});
            }}>
              {this.ExpansionPanel?<>
                <i className='iconfont'>&#xe650;</i>{i18n.t('Collapse')} 
              </>:<>
                <i className='iconfont'>&#xe734;</i>{i18n.t('Expand')} 
              </>}
            </a>
            {/* <a className='grey1' onClick={(e) => this.close_transter_dialog('local')}>
              <i className='iconfont'>&#xe65d;</i>{i18n.t('Transfer')}({this._num_local})</a> */}
          </div>
        </div>

        <div className={this.ExpansionPanel?'bottom2':'bottom2-2'}
          // draggable="true" 
          onDrop={this.ondrop_local}
          onDragEnter={(e) => e.preventDefault()} 
          onDragOver={(e) => e.preventDefault()}
          onClick={(e) => setTimeout(() => this.onclickpancel_fromlocal(e), 50)}
          onMouseUp={(e) => setTimeout(() => this.onmouseuppancel_fromlocal(e), 60)}>
          <div className='filearea' onContextMenu={this.onContextMenuLocal} ref={this.filearea2_ref}>
            {this.SelectedFilesLocal.map((v: any, i: number) => {
              return <Main1DirItemLocalV2
                path={this.CurPathLocal}
                opendir={this.opendir_local}
                fileobj={v}
                oncontextmenu={(e) => this.oncontextmenu_local(i, e)}
                onclick={() => setTimeout(() => this.onselect_fromlocal(i), 10)}
                select={v.select == 1 ? true : false}
                ondragend={(e) => {}}
                ondragstart={(e) => this.dragstart_item_local(i, e)}
                key={i} />
            })}
            <div style={{ height: "50px", width: "100%" }}></div>
          </div>
          {this.FileTransferLocalState == 1 && <>
            <div className='transferlist'>
              <div className='progress head'>
                <a onClick={() => this.stop_transfer('local')}><i className='iconfont'>&#xe61a;</i></a>
              </div>
              <FileTransferProgressV2
                color='#e86b00'
                filename={this._filename_local}
                size={this._size_local}
                count={this._count_local}
                num={this._num_local}
                nums={this._nums_local} />
            </div>
          </>}
        </div>

      </div>
    )
  }

}

export default Main1DirPanelV2;
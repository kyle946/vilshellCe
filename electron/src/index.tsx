
import * as React from 'react'
import { type PropsWithChildren, useState, useRef, useEffect } from 'react'
import * as ReactDOM from 'react-dom'
import {
  Route, HashRouter as Router,
  Switch
} from 'react-router-dom'
import path from "path";
import i18n from 'i18next';
import { ipcRenderer } from "electron";
import { createHashHistory, LocationDescriptorObject, History } from "history";
import { Toast, Button, Empty } from '@douyinfe/semi-ui';
const history = createHashHistory();
import { yxulinux_connect_state, yxulinux_file_type as ft, yxulinux_error_code as cc } from "./types/ErrorCode";
import Loading, { loadingdlg, closeloadingdlg } from "./components/Loading";

import Wintitle from './components/Wintitle';
import * as Common from "./components/Common";
import { loadlang } from "./locales/index"
import LaunchHome from "./pages/LaunchHome"
import Main1 from "./pages/Main1"
import * as dlg from "./components/Dialog";


import indexevt from "./indexevt"
import "./index.css"
import villib1 from "@/../dist/villib1.node"

global.apphandle = new villib1.apphandle();

/**
 * 初始化全局变量
 */
global.vilssh = null;
global.uilang = null;
global.savestate1 = React.createRef<HTMLSpanElement>();
global.alert2 = dlg.alert;
global.confirm2 = dlg.confirm;
global.toast2 = Toast;
global.ievt = new indexevt();



global.DIRSEPA = "";
if (process.platform == 'darwin') {
  global.DIRSEPA = '/';
}
else if (process.platform == 'win32') {
  global.DIRSEPA = '\\';
}

window.vildoc = {
  closewin: function () {
    return ipcRenderer.invoke('vildoc.closewin');
  },
  minwin: function () {
    return ipcRenderer.invoke('vildoc.minwin');
  },
  maxwin: function () {
    return ipcRenderer.invoke('vildoc.maxwin');
  },
  restore: function () {
    return ipcRenderer.invoke('vildoc.restore');
  },
  gethomepath: function (name: string) {
    return ipcRenderer.invoke('vildoc.gethomepath', name);
  },
  choosedirectory: function (title: string) {
    return ipcRenderer.invoke('vildoc.choosedirectory', title);
  },
  savefile: function (title: string) {
    return ipcRenderer.invoke('vildoc.savefile', title);
  },
  choosefile: function (title: string) {
    return ipcRenderer.invoke('vildoc.choosefile', title);
  },
  jumpurl: function (url: string) {
    return ipcRenderer.invoke('vildoc.jumpurl', url);
  },
  showMessageBoxSync: function (param: any) {
    return ipcRenderer.invoke('vildoc.showMessageBoxSync', param);
  },
  geturl: function (param: any) {
    return ipcRenderer.invoke('vildoc.geturl', param);
  },
  gettabs: function(...args: any){
    return ipcRenderer.sendSync('vildoc.gettabs', args)
  },
  onfocus: function(...args: any){
    return ipcRenderer.invoke('vildoc.onfocus', args)
  },
  mac_win_bug: function(...args: any){
    return ipcRenderer.invoke('vildoc.mac_win_bug', args)
  },
  ///
  //==
  ///
  handle_test: function(...args: any){
    return ipcRenderer.invoke('vildoc.handle_test', args)
  },
  on_test: function(...args: any){
    return ipcRenderer.sendSync('vildoc.on_test', args)
  }
}

ipcRenderer.on('close_tab', (event: any, value: any)=>{
  // 关闭 ssh连接
  Common.closeConnReleaseConn();
});


/**
 * 如果在开发模式下，打开F12开发者工具栏快捷键
 */
if (process.env.NODE_ENV == "development") {
  document.onkeydown = (e) => {
    if (e.key == 'F12') {
      ipcRenderer.send("openDevtools", true)
    }
    else if ((e.key == 't' || e.key == 'T') && e.ctrlKey) {
      //
    }
    else if ((e.key == 'q' || e.key == 'Q') && e.ctrlKey) {
      //
    }
  }
}




class Index extends React.Component<any,any> {

  public _loadref = React.createRef<HTMLDivElement>();

  constructor(props:any){
    super(props);
    this.showload = this.showload.bind(this);
    this.closeload = this.closeload.bind(this);
    this.state={
      rand: 0.1,
      loadtxt: "Loading."
    }
  }

  componentDidMount(): void {
    global.ievt.addListener("showload", this.showload);
    global.ievt.addListener("closeload", this.closeload);
  }

  componentWillUnmount(): void {
    global.ievt.removeListener("showload", this.showload);
    global.ievt.removeListener("closeload", this.closeload);
  }
  

  showload(txt:string){
    if(!txt){
      txt="Loading.";
    }
    this.setState({loadtxt:txt});
    this._loadref.current.style.height = "calc(100vh - 1px)";
  }

  closeload(){
    this._loadref.current.style.height = "0px";
  }

  render(): React.ReactNode {
    return (
      <div className='Index'>
        <div className='loaddlg' ref={this._loadref}><Loading txt={this.state.loadtxt} /></div>
        <Wintitle />
        <Router basename="/">
          <Switch>
            <Route path="/" exact component={LaunchHome}></Route>
            <Route path="/main1" component={Main1}></Route>
            <Route path="/launchhome" component={LaunchHome}></Route>
          </Switch>
        </Router>
      </div>
    );
  }
}


loadlang().then(() => {
  ReactDOM.render(<Index />, document.getElementById('app'));
});
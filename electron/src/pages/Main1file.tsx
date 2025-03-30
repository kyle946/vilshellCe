import * as React from 'react'
import i18n from 'i18next';
import { Trans, Translation, withTranslation } from 'react-i18next'
import { type PropsWithChildren, useState, useRef, useEffect } from 'react'
import * as ReactDOM from 'react-dom'
import { createHashHistory, LocationDescriptorObject, History } from "history";
const history = createHashHistory();
import fs from "fs"
import path from "path"
import EventEmitter from "events";
//===============

import { Tabs, Spin, TabPane, Button, Progress } from '@douyinfe/semi-ui';


import * as Common from "../components/Common";
import { yxulinux_connect_state, yxulinux_file_type as ft, yxulinux_error_code as cc } from "../types/ErrorCode";
import * as dlg from "../components/Dialog";
import { loadingdlg, closeloadingdlg } from "../components/Loading";
import { confirmdlg, rm_confirmdlg } from "../components/ConfirmDialog";
import { PopupMenudlg, type PopupMenuItem } from "../components/PopupMenu"

import "./Main1file.less"
import { KeyboardEvent } from 'electron';
import Main1DirPanelV2 from "./Main1DirPanelV2";





type Main1fileTabPancelStruct = {
  tab: string,                      // 标签页栏显示文字
  itemKey: string,                  // 对应 activeKey
  closable: boolean,                // 是否允许关闭
  path_server: string,              // 服务器目录
  path_local: string,               // 本地目录
  filetype: string,                 // 类型，是文件夹(dir)还是编辑器(file)
};
type Main1file1Props = PropsWithChildren<{}>;
class Main1file extends React.Component<Main1file1Props>{

  public defaulttab:string; 
  public newTabIndex:number;
  public active:string; 
  public tabs: Main1fileTabPancelStruct[]; 
  public defaultpath: string[];

  constructor(props: Main1file1Props){
    super(props);

    this.onclose=this.onclose.bind(this);
    this.onchange=this.onchange.bind(this);
    this.newtab=this.newtab.bind(this);
    this.addtab=this.addtab.bind(this);
    this.load_default_dir=this.load_default_dir.bind(this);
    this.renderTabBarExtraContent=this.renderTabBarExtraContent.bind(this);

    this.defaulttab = "default";
    this.newTabIndex=0;
    this.active = this.defaulttab;
    this.tabs = [];
    this.defaultpath = ["", ""];
    this.state={
      rand: 0.1
    };
  }

  componentDidMount(): void {
    global.ievt.addListener("newtab", this.newtab);
    this.load_default_dir();
  }

  componentWillUnmount(): void {
    global.ievt.removeListener("newtab", this.newtab);
  }

  /**
   * 关闭标签
   * @param key 
   */
  onclose(key: string){
    const _index = this.tabs.findIndex((t: any) => t.itemKey === key);
    this.tabs.splice(_index, 1);
    let _item = this.tabs[this.tabs.length - 1];
    this.active = _item ? _item.itemKey : "";
    this.setState({rand: Math.random()});
  }

  /**
   * 切换标签
   * @param key 
   */
  onchange(key: string){
    this.active = key;
    this.setState({rand: Math.random()});
  }

  /**
   * 添加新标签
   * @param path_server 
   * @param path_local 
   * @param filetype 
   * @param closable 
   * @returns 
   */
  newtab(path_server: string, path_local: string, filetype: string, closable = true){
    /// 文件夹
    if(filetype=="dir"){
    }
    else if(filetype=="file"){
      let ret:number = global.vilssh.readfilecheck(path_server);
      if(ret!=cc.YXU_SUCCESS){
        dlg.alert(i18n.t('error'), i18n.t(`error_code_${ret}`));
        return ;
      }
    }
    if (path_server == "") {
      path_server = this.defaultpath[0];
    }
    if (path_local == "") {
      path_local = this.defaultpath[1];
    }
    let tab = path.basename(path_server);
    let index = this.newTabIndex++;
    let itemKey = `newtab${index}`;
    let pancel: Main1fileTabPancelStruct = {
      tab,                            // 标签页栏显示文字
      itemKey,                        // 对应 activeKey
      closable,                       // 是否允许关闭
      path_server,                    // 服务器目录
      path_local,                     // 本地目录
      filetype,                       // 类型，是文件夹(ft.YXU_S_IFDIR)还是编辑器(ft.YXU_S_IFREG)
    };
    this.tabs.push(pancel);
    this.active=itemKey;
    this.setState({rand: Math.random()});
  }

  /**
   * 添加新标签
   */
  addtab(){
    this.newtab(this.defaultpath[0], this.defaultpath[1], "dir");
  };

  /**
   * 服务器目录面板，目录地址被改变
   * @param dir 
   * @param itemKey 
   */
  onChangeDirSer(dir:string, itemKey:string){
    const _index = this.tabs.findIndex((t: any) => t.itemKey === itemKey);
    this.tabs[_index].tab=dir;
    this.setState({rand: Math.random()});
  }

  /**
   * 加载记住的目录
   */
  async load_default_dir(){

    this.defaultpath = [global.vilssh.hostinfo.homeuserdir, apphandle.appinfo.userhome];
    this.setState({rand: Math.random()});

    let filename = global.vilssh.hostinfo.hostlink+".ini";
    let hostini = apphandle.appinfo.vilshelldir + global.DIRSEPA + filename;

    let loaddirs: Map<string, string> = new Map();
    loaddirs.set(this.defaultpath[0], this.defaultpath[1]);
    if (fs.existsSync(hostini)) {
      let confjsonstr = apphandle.readfile(hostini);
      try {
        let confjson;
        confjson = JSON.parse(confjsonstr);
        loaddirs = new Map(confjson);
      } catch (error) {
        console.log("json error", error);
      }
    }
    loaddirs.forEach((v, k) => {
      this.newtab(k, v, "dir", true);
    });
  }

  //
  onSetting(){
    //
  }

  renderTabBarExtraContent(){
    return (
      <div className='extracontent'>
        <a onClick={this.addtab}>{i18n.t('New Tab')}</a>
      </div>
    )
  }

  render(){
    return (
      <div className='Main1file1'>
        <Tabs
          type="card"
          defaultActiveKey={this.defaulttab}
          tabPaneMotion={false}
          onTabClose={this.onclose}
          activeKey={this.active}
          onChange={this.onchange}
          tabBarExtraContent={this.renderTabBarExtraContent()}>
          {this.tabs.map((t, index) => (
            <TabPane closable={t.closable} tab={t.tab||"~~"} itemKey={t.itemKey} key={index}>
              {t.filetype == "dir" && <>
                <Main1DirPanelV2 itemKey={t.itemKey}
                  ChangeDir={(dir)=>this.onChangeDirSer(dir,t.itemKey)}
                  path_server={t.path_server}
                  path_local={t.path_local}
                  filetype={t.filetype} />
              </>}
            </TabPane>
          ))}
        </Tabs>
      </div>
    );
  }
}

export default Main1file;
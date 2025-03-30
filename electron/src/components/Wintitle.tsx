
import * as React from 'react'
import { type PropsWithChildren, useState, useRef, useEffect } from 'react'
import i18n from 'i18next';
import * as ReactDOM from 'react-dom'
import { createHashHistory, LocationDescriptorObject, History } from "history";
const history = createHashHistory();
//----------------------------------------
import logo1 from "../assets/logo1.png";
import * as dlg from "./Dialog";
import { SwitchLang1dlg, closeSwitchLang1dlg } from "../pages/SwitchLang";
import CreateConn from "../pages/CreateConn"
import * as Common from "../components/Common";
import { yxulinux_connect_state, yxulinux_error_code as cc } from "../types/ErrorCode";
import "./Wintitle.less"

import villib1 from "@/../dist/villib1.node";



export default class Wintitle extends React.Component<any,any> {

  public newtabindex = 0;
  constructor(props:any){
    super(props);
    this.onSwitchLang=this.onSwitchLang.bind(this);
    this.sethoststring=this.sethoststring.bind(this);
  }

  state = {
    hoststring: ""
  }
  
  componentDidMount(): void {
    global.ievt.addListener('sethoststring', this.sethoststring);
  }

  componentWillUnmount(): void {
    global.ievt.removeListener('sethoststring', this.sethoststring);
  }

  sethoststring(val:string){
    this.setState({hoststring: val});
    if(val!=""){
      let wintitle = val + "|" + i18n.t('apptitle');
      document.title = wintitle;
    }else{
      let wintitle = i18n.t('apptitle');
      document.title = wintitle;
    }
  }
  
  onSwitchLang () {
    SwitchLang1dlg();
  }
  

  render(): React.ReactNode {
    return (
      <div className="wintitle">
        <div className='wintitle2'>
          <div className="linadefault">
            <div className='logo1'><img src={logo1} /></div>
            <a>{i18n.t('apptitle')}</a>
          </div>
          {this.state.hoststring!=""&&<div className='wintitle3'> | {this.state.hoststring}</div>}
        </div>
        <div className='wintitle_c2_icon'>
          <a className='wintitle_c2_switchlang' onClick={this.onSwitchLang}><i className="iconfont">&#xeb67;</i><span>{i18n.t('Lang')}</span></a>
          <a onClick={() => global.vildoc.minwin()}><i className="iconfont">&#xe67a;</i></a>
          <a onClick={() => global.vildoc.maxwin()}><i className="iconfont">&#xe60d;</i></a>
          <a onClick={() => global.vildoc.closewin()}><i className="iconfont">&#xe61a;</i></a>
        </div>
      </div>
    )
  }
}
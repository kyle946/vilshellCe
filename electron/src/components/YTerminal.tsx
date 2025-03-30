import * as React from 'react';
import i18n from 'i18next';
import { Trans, Translation, withTranslation } from 'react-i18next';
import { type PropsWithChildren, useState, useRef, useEffect } from 'react';
import * as ReactDOM from 'react-dom';
import { createHashHistory, LocationDescriptorObject, History } from "history";
const history = createHashHistory();
import fs from "fs";
import path from "path";
//===============

import "@xterm/xterm/css/xterm.css";
import { Terminal } from '@xterm/xterm';
import { FitAddon } from "@xterm/addon-fit";

import { Tabs, TabPane, RadioGroup, Radio, Empty, Button } from '@douyinfe/semi-ui';
import { IconFile, IconGlobe, IconHelpCircle } from '@douyinfe/semi-icons';
import { IllustrationConstruction, IllustrationConstructionDark } from '@douyinfe/semi-illustrations'


import * as Common from "./Common";
import { yxulinux_connect_state, yxulinux_file_type as ft, yxulinux_error_code as cc } from "../types/ErrorCode";
import * as dlg from "./Dialog";
import Loading, { loadingdlg, closeloadingdlg } from "./Loading";
import { confirmdlg, rm_confirmdlg } from "./ConfirmDialog";
import { PopupMenudlg, type PopupMenuItem } from "./PopupMenu"


import "./YTerminal.less"
import villib1 from "@/../dist/villib1.node";


type YTerminalProps = PropsWithChildren<{
  label: string,
  firstCmd?: string,
  el:HTMLDivElement|HTMLElement
}>

function YTerminalTer (props: any) {

  let term: Terminal = new Terminal({
    fontSize: 12,
    cols: 110,
    rows: 20,
    theme: {
      background: '#333', // 设置背景颜色
      foreground: '#ffffff', // 设置前景颜色为白色 
      cursor: '#ffffff', // 光标颜色
      black: '#000000',
      red: '#cd3131',
      green: '#0dbc79',
      yellow: '#e5e510',
      blue: '#2472c8',
      magenta: '#bc3fbc',
      cyan: '#11a8cd',
      white: '#e5e5e5',
      brightBlack: '#666666',
      brightRed: '#f14c4c',
      brightGreen: '#23d18b',
      brightYellow: '#f5f543',
      brightBlue: '#3b8eea',
      brightMagenta: '#d670d6',
      brightCyan: '#29b8db',
      brightWhite: '#ffffff'
    }
  });
  let term_fit: FitAddon = new FitAddon();
  let termref: React.RefObject<HTMLDivElement> = React.createRef<HTMLDivElement>();

  function startup(){
    term.open(termref.current);
    term.loadAddon(term_fit);
    term_fit.fit();
    term.focus();
    term.onData((data) => {
      global.vilssh.chanelwrite(props.label, "" + data);
    });
    try {
      apphandle.loginfo("chanelnew", props.label+","+term.cols+","+term.rows);
      global.vilssh.chanelnew(props.label, term.cols, term.rows, (out: any)=>{
        term.write(out);
      }, (out:number)=>{
        if (out == cc.YXU_SUCCESS) {
          // 连接成功
          if (props.firstcmd != "") global.vilssh.chanelwrite(props.label, props.firstcmd + "\r");
        } 
        else if (out == cc.YXU_READ_CHANNEL_FAILED) {
          // 消息读取失败
          apphandle.logerr("chanelnew","read message failed.");
        }
        else if (out == cc.YXU_READ_SUCCESS) {
          // 消息读取成功
          apphandle.loginfo("chanelnew","read message successed.");
        }
        else {
          // 连接失败
          dlg.alert(i18n.t('error'), i18n.t(`error_code_${out}`));
          // term.writeln('');
          // term.write("Please enter the password to complete the authentication again.");
        }
      });
    } catch (error) {
      console.error(error);
    }
  }

  useEffect(() => {
    // 没有连接主机
    if (!Common.isConnHost()) {
      return;
    }
    startup();
    return () => {
      global.vilssh.chanelclose(props.label); // 关闭终端
    };
  }, [])
  return (
    <div className='term' ref={termref}></div>
  );
};





class YTerminalTer2 extends React.Component<any, any>{
  public term_fit:FitAddon; 
  public termref:React.RefObject<HTMLDivElement>; 
  public term: Terminal;
  constructor(props:any){
    super(props);
    this.sendcmd=this.sendcmd.bind(this);
    this.term_fit = new FitAddon();
    this.termref = React.createRef<HTMLDivElement>();
    this.term = new Terminal({
      fontSize: 12,
      cols: 110,
      rows: 20,
      theme: {
        background: '#333', // 设置背景颜色
        foreground: '#ffffff', // 设置前景颜色为白色 
        cursor: '#ffffff', // 光标颜色
        black: '#000000',
        red: '#cd3131',
        green: '#0dbc79',
        yellow: '#e5e510',
        blue: '#2472c8',
        magenta: '#bc3fbc',
        cyan: '#11a8cd',
        white: '#e5e5e5',
        brightBlack: '#666666',
        brightRed: '#f14c4c',
        brightGreen: '#23d18b',
        brightYellow: '#f5f543',
        brightBlue: '#3b8eea',
        brightMagenta: '#d670d6',
        brightCyan: '#29b8db',
        brightWhite: '#ffffff'
      }
    });
  }
  componentDidMount(): void {
    if (!Common.isConnHost()) return;
    this.term.open(this.termref.current);
    this.term.loadAddon(this.term_fit);
    this.term_fit.fit();
    this.term.focus();
    this.term.onData((data) => {
      global.vilssh.chanelwrite(this.props.label, "" + data);
    });
    try {
      global.vilssh.chanelnew(this.props.label, this.term.cols, this.term.rows, (out: any)=>{
        this.term.write(out);
      }, (out:number)=>{
        if (out == cc.YXU_SUCCESS) {
          // 连接成功
          if (this.props.firstcmd != "") global.vilssh.chanelwrite(this.props.label, this.props.firstcmd + "\r");
        } 
        else if (out == cc.YXU_READ_CHANNEL_FAILED) {
          // 消息读取失败
          apphandle.logerr("chanelnew","read message failed.");
        }
        else if (out == cc.YXU_READ_SUCCESS) {
          // 消息读取成功
          apphandle.loginfo("chanelnew","read message successed.");
        }
        else {
          // 连接失败
          dlg.alert(i18n.t('error'), i18n.t(`error_code_${out}`));
          // term.writeln('');
          // term.write("Please enter the password to complete the authentication again.");
        }
      });
    } catch (error) {
      console.error(error);
    }
  }
  componentWillUnmount(): void {
    if (!Common.isConnHost()) return;
    global.vilssh.chanelclose(this.props.label); // 关闭终端
  }
  sendcmd(cmd:string){
    global.vilssh.chanelwrite(this.props.label, cmd + "\r");
  }
  render(){
    return <div className='term' ref={this.termref}></div>
  }
}



export class YTerminal2 extends React.Component<YTerminalProps, any>{

  constructor(props: YTerminalProps) {
    super(props);
    this.close = this.close.bind(this);
    this.add = this.add.bind(this);
    this.onChange = this.onChange.bind(this);
    this.get_tab_index = this.get_tab_index.bind(this);
    ///
    this.state = {
      activeKey: this.props.label,
      panes: [
        { 
          tab: this.props.label, 
          itemKey: this.props.label, 
          closable: false, 
          firstcmd: this.props.firstCmd,
          ref: React.createRef<any>()
         },
      ]
    };
  }

  componentDidMount(): void {  }


  get_tab_index(itemKey: any){
    return this.state.panes.findIndex((t: any) => t.itemKey === itemKey);
  }

  close(itemKey: any) {
    var { panes } = this.state;
    const closeIndex = panes.findIndex((t: any) => t.itemKey === itemKey);
    panes.splice(closeIndex, 1);
    let _item = panes[panes.length - 1];
    let _itemkey = _item ? _item.itemKey : "";
    this.setState({ panes, activeKey: _itemkey });
  }

  add(itemKey: string, firstcmd: string) {
    const { panes } = this.state;
    panes.push({ tab: itemKey, itemKey, closable: true, firstcmd, ref: React.createRef<any>() });
    this.setState({ panes, activeKey: itemKey });
  }

  onChange(activeKey: string) {
    this.setState({ activeKey });
  }

  showtab(activeKey: string, cmd: string){
    this.setState({ activeKey }, ()=>{
      if(cmd!=""){
        var { panes } = this.state;
        const ii = panes.findIndex((t: any) => t.itemKey === activeKey);
        panes[ii].ref.current.sendcmd(cmd);
      }
    });
  }

  onExit(){
    let el: HTMLElement = this.props.el;
    el.setAttribute('open', '0');
    ReactDOM.unmountComponentAtNode(el);
  }

  onMin(){
    let el: HTMLElement = this.props.el;
    if(el.getAttribute('min')=='1'){
      el.setAttribute('min', '0');
      el.style.height="auto";
      el.style.width="auto";
      el.style.overflow="auto";
      el.style.overflowX="hidden";
      el.style.left="0px";
      el.style.bottom="0px";
    }else{
      el.style.height="22px";
      el.style.width="150px";
      el.style.overflow="hidden";
      el.style.left="0px";
      el.style.bottom="0px";
      el.setAttribute('min', '1');
    }
  }

  renderTabBarExtraContent() {
    return (
      <div className='closebtn'>
        <a onClick={() => this.onMin()}><i className='iconfont'>&#xe60f;</i></a>
        <a onClick={() => this.onExit()}><i className='iconfont'>&#xe8bb;</i></a>
      </div>
    )
  }

  render(): React.ReactNode {
    return (
      <div id='Terminal_1'>
        <Tabs
          type="card"
          defaultActiveKey={this.props.label}
          tabPaneMotion={false}
          onTabClose={this.close}
          activeKey={this.state.activeKey}
          onChange={this.onChange}
          tabBarExtraContent={this.renderTabBarExtraContent()}>
          {this.state.panes.map((t: any) => (
            <TabPane closable={t.closable} tab={t.tab} itemKey={t.itemKey} key={t.itemKey}>
              <YTerminalTer2 label={t.itemKey} firstcmd={t.firstcmd} ref={t.ref} />
            </TabPane>
          ))}
        </Tabs>
      </div>
    )
  }
}


export function open_terminal(label:string, firstCmd:string, el:HTMLDivElement|HTMLElement, ref: React.RefObject<YTerminal2> = null){
  if(el.getAttribute('open')=='1'){
    el.style.height="auto";
    el.style.width="auto";
    el.style.left="0px";
    el.style.bottom="0px";
    el.setAttribute('min', '0');
    if(ref!=null){
      if(ref.current.get_tab_index(label)>=0){
        ref.current.showtab(label, firstCmd);
      }else{
        ref.current.add(label, firstCmd);
      }
    }
  }else{
    el.setAttribute('open', '1');
    el.setAttribute('min', '0');
    if(ref==null){
      ReactDOM.render(<YTerminal2 label={label} firstCmd={firstCmd} el={el} />, el);
    }else{
      ReactDOM.render(<YTerminal2 label={label} firstCmd={firstCmd} el={el} ref={ref} />, el);
    }
  }
}

export function close_terminal(el: HTMLDivElement){
  el.setAttribute('open', '0');
  ReactDOM.unmountComponentAtNode(el);
  // document.removeChild(el);
}

export function create_element(id: string){
  let el:HTMLElement = document.getElementById(id);
  let ele = document.createElement("div");
  ele.style.position = 'absolute';
  ele.style.left = "10vw";
  ele.style.top = "10vh";
  ele.style.width = '1px';
  ele.style.height = '1px';
  el.appendChild(ele);
}

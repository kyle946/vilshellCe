import * as React from 'react'
import i18n from 'i18next';
import { Trans, Translation, withTranslation } from 'react-i18next'
import { type PropsWithChildren, useState, useRef, useEffect } from 'react'
import * as ReactDOM from 'react-dom'
import { createHashHistory, LocationDescriptorObject, History } from "history";
const history = createHashHistory();
import fs from "fs"
import path from "path"
//===============
import "@xterm/xterm/css/xterm.css";
import { Terminal } from '@xterm/xterm';
import { FitAddon } from "@xterm/addon-fit";
import { Tabs, Spin, TabPane, Button } from '@douyinfe/semi-ui';

;
import * as Common from "../components/Common";
import { yxulinux_connect_state, yxulinux_file_type as ft, yxulinux_error_code as cc } from "../types/ErrorCode";
import * as dlg from "../components/Dialog";
import { loadingdlg, closeloadingdlg } from "../components/Loading";
import { confirmdlg, rm_confirmdlg } from "../components/ConfirmDialog";
import { PopupMenudlg, type PopupMenuItem } from "../components/PopupMenu"


import "./Main1term.less"
import villib1 from "@/../dist/villib1.node";




type Main1TerminalProps = PropsWithChildren<{
  label: string,
  defaultpath: string
}>;
const Main1Terminal: React.FC<Main1TerminalProps> = (props: Main1TerminalProps) => {

  let term: Terminal = new Terminal({
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

  function winresize(){
    try {
      // 没有连接主机
      if (!Common.isConnHost()) {
        return;
      }
      // 窗口大小改变时，触发xterm的resize方法使自适应
      term_fit.fit();
      global.vilssh.chanelresize(props.label, term.cols, term.rows);
    } catch (e) {
      console.log("e", e.message)
    }
  }

  function startup(){
    term.open(termref.current);
    term.loadAddon(term_fit);
    term_fit.fit();
    term.focus();
    term.onData((data) => {
      global.vilssh.chanelwrite(props.label, "" + data);
    });
    generage_channel();
    ///
    setTimeout(() => {
      winresize();
    }, 800);
  }

  // 获取缓冲区内容
  function getTerminalText() {
    const buffer = term.buffer.active;
    let text = '';
    for (let i = 0; i < buffer.length; i++) {
      text += buffer.getLine(i).translateToString() + '\n';
    }
    return text;
  }

  function generage_channel(){
    ///
    let onRecv=(out: any)=>{
      term.write(out);
      // console.log(getTerminalText());
    };
    ///
    let onError=(out:number)=>{
      if (out == cc.YXU_SUCCESS) {
        // 连接成功
        if (props.defaultpath != "") global.vilssh.chanelwrite(props.label, "cd " + props.defaultpath + "\r");
      } 
      else if (out == cc.YXU_READ_CHANNEL_FAILED) {
        // 消息读取失败
      }
      else if (out == cc.YXU_READ_SUCCESS) {
        // 消息读取成功
      }
      else {
        // 连接失败
        dlg.alert(i18n.t('error'), i18n.t(`error_code_${out}`));
        term.writeln('');
        term.write("Please enter the password to complete the authentication again.");
      }
    };
    ///
    global.vilssh.chanelnew(props.label, term.cols, term.rows, onRecv, onError);
  };

  useEffect(() => {
    // 没有连接主机
    if (!Common.isConnHost()) {
      return;
    }
    window.addEventListener("resize", winresize);
    startup();
    return () => {
      window.removeEventListener("resize", winresize);
      global.vilssh.chanelclose(props.label); // 关闭终端
    };
  }, [])

  return (
    <div className='termdivele' ref={termref}></div>
  );
};

/******************************************************************
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 ******************************************************************/

type IProps = PropsWithChildren<{
  onPress?: () => void
}>;
class Main1term extends React.Component<IProps, any> {

  public newTabIndex = 0;
  public readonly defaulttab: string = "default";

  constructor(props: IProps) {
    super(props);
    this.close = this.close.bind(this);
    this.add = this.add.bind(this);
    this.onchange = this.onchange.bind(this);
    this.state = {
      activeKey: this.defaulttab,
      panes: [
        { tab: this.defaulttab, itemKey: this.defaulttab, closable: true, defaultpath: "" },
      ]
    };
  }

  componentDidMount(): void {
    global.ievt.addListener("newterm", this.add);
  }

  close(key: any) {
    var { panes } = this.state;
    const closeIndex = panes.findIndex((t: any) => t.itemKey === key);
    panes.splice(closeIndex, 1);
    let _item = panes[panes.length - 1];
    let _itemkey = _item ? _item.itemKey : "";
    this.setState({ panes, activeKey: _itemkey });
  }

  add(defaultpath: string) {
    const { panes } = this.state;
    const index = this.newTabIndex++;
    panes.push({ tab: `newtab${index}`, itemKey: `newtab${index}`, closable: true, defaultpath });
    this.setState({ panes, activeKey: `newtab${index}` });
  }

  onchange(activeKey: string) {
    this.setState({ activeKey });
  }

  renderTabBarExtraContent() {
    return (
      <div className='extracontent'>
        <span>Shift+Ins({i18n.t('Paster')}) , Ctrl+Ins({i18n.t('Copy')})</span>
        <a onClick={() => this.add("")}>{i18n.t('New Terminal')}</a>
      </div>
    )
  }

  render(): React.ReactNode {
    return (
      <div className='Main1term1'>
        <Tabs
          type="card"
          defaultActiveKey={this.defaulttab}
          tabPaneMotion={false}
          onTabClose={this.close.bind(this)}
          activeKey={this.state.activeKey}
          onChange={this.onchange}
          tabBarExtraContent={this.renderTabBarExtraContent()}>
          {this.state.panes.map((t: any) => (
            <TabPane closable={t.closable} tab={t.tab} itemKey={t.itemKey} key={t.itemKey}>
              <Main1Terminal label={t.itemKey} defaultpath={t.defaultpath} />
            </TabPane>
          ))}
        </Tabs>
      </div>
    )
  }

}


export default Main1term;

import * as React from 'react';
import i18n from 'i18next';
import { Trans, Translation, withTranslation } from 'react-i18next';
import { type PropsWithChildren, useState, useRef, useEffect } from 'react';
import * as ReactDOM from 'react-dom';
import { createHashHistory, LocationDescriptorObject, History } from "history";
const history = createHashHistory();
import Main1term from './Main1term'
import Main1file from './Main1file'
import Main1Menu from './Main1Menu'

import "./Main1.less"
import villib1 from "@/../dist/villib1.node";







type IProps = PropsWithChildren<{
  onPress?: () => void
}>;
class MainPage extends React.Component<IProps, any> {

  public defaultmenu:string = "term";
  public refterm: React.RefObject<HTMLDivElement> = React.createRef<HTMLDivElement>();
  public reffile: React.RefObject<HTMLDivElement> = React.createRef<HTMLDivElement>();
  public Main1MenuData = [
    { label: "term", text: i18n.t("Terminal"), icon: "\ue663" },
    { label: "file", text: i18n.t('File'), icon: "\ue614" },
  ];

  constructor(props: IProps) {
    super(props);
    this.onclickmenu = this.onclickmenu.bind(this);
  }

  componentDidMount(): void {
    global.ievt.addListener("onclickmenu", this.onclickmenu);
    /// 显示终端
    this.onclickmenu("term");
  }

  componentWillUnmount(): void {
    global.ievt.removeListener("onclickmenu", this.onclickmenu);
  }

  onclickmenu(label:string){
    if(label=="term"){
      if(this.refterm.current.getAttribute("data-init")=="0"){
        ReactDOM.render(<Main1term />, this.refterm.current);
        this.refterm.current.setAttribute("data-init", "1");
      }
      {
        this.refterm.current.style.height='auto';
        this.reffile.current.style.height='0px';
        this.reffile.current.style.overflow="hidden";
      }
    }
    else if(label=="file"){
      if(this.reffile.current.getAttribute("data-init")=="0"){
        ReactDOM.render(<Main1file />, this.reffile.current);
        this.reffile.current.setAttribute("data-init", "1");
      }
      {
        this.reffile.current.style.height='auto';
        this.refterm.current.style.height='0px';
        this.refterm.current.style.overflow="hidden";
      }
    }
  }

  render(): React.ReactNode {
    return (
      <div className='MainPage1'>
        <Main1Menu default={this.defaultmenu} onclick={this.onclickmenu} menu={this.Main1MenuData} />
        <div className='MainPage2'>
          <div className='term' ref={this.refterm} data-init="0"></div>
          <div className='file' ref={this.reffile} data-init="0"></div>
        </div>
      </div>
    )
  }

}


export default MainPage;

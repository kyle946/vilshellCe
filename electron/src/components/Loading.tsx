
import * as React from 'react'
import { type PropsWithChildren, useState, useRef, useEffect } from 'react'
import * as ReactDOM from 'react-dom'
import i18n from 'i18next';
//-------------------------------
import * as Common from "../components/Common";
import { Select, Spin } from '@douyinfe/semi-ui';
import "./Loading.css"
import villib1 from "@/../dist/villib1.node";


type LoadingProps = PropsWithChildren<{
  txt?: string
}>;
class Loading extends React.Component<any, any>{


  constructor(props: any) {
    super(props);
  }

  componentDidMount(): void { }

  closeDlg(){
    global.ievt.emit('closeload');
  }

  render(): React.ReactNode {
    return (
      <div className='loading'>
        <div className='loading_bg'></div>
        <div className='loading_win'>
          <div className='loading_close'><a onClick={() => this.closeDlg()}><i className='iconfont'>&#xe8bb;</i></a></div>
          <div className='loading_content'>
            <div className='loading_txt'>{this.props.txt}</div>
            <Spin size="large" />
          </div>
        </div>
      </div>
    )
  }
}

const closeloadingdlg = () => {
  global.ievt.emit('closeload');
}

const loadingdlg = () => {
  global.ievt.emit('showload',"");
}

export { loadingdlg, closeloadingdlg }
export default Loading;
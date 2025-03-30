import * as React from 'react'
import i18n from 'i18next';
import { Trans, Translation, withTranslation } from 'react-i18next'
import * as ReactDOM from 'react-dom'
import { createHashHistory, LocationDescriptorObject, History } from "history";
const history = createHashHistory();


import * as Common from "../components/Common";
import "./LaunchHome.less"
import Wintitle from "../components/Wintitle"
import CreateConn, { closeconndlg, CreateConndlg } from "./CreateConn"
import { alert, confirm, confirminput } from "../components/Dialog";
import { yxulinux_connect_state, yxulinux_error_code } from "../types/ErrorCode";
import * as dlg from "../components/Dialog";


import villib1 from "@/../dist/villib1.node";




class LaunchHome extends React.Component<any, any> {

  constructor(props: any) {
    super(props);
    this.startup = this.startup.bind(this);
  }

  componentDidMount(): void {
    this.startup();
  }

  async startup() {
    if (!Common.isConnHost()) {
      global.vilssh = new villib1.vilssh();
    }
    if(global.vilssh.conn!=yxulinux_connect_state.YXU_CONNECT_AUTH){
      CreateConndlg();
    }
    // let argv1 = await global.vildoc.argv(1);
  }

  onNewConn(){
    CreateConndlg();
  }

  render(): React.ReactNode {
    return (
      <>
        <div className='homelaunch1' >
          <div className='area1'>
            <div className='area1_1'>
              <div className='area1_4'>
                <a onClick={this.onNewConn}><i className='iconfont'>&#xe663;</i>{i18n.t('New Connection')}</a>
              </div>
              <div className='area1_3'>
                <div className='h10'></div>
                <a className='a2' onClick={()=>global.vildoc.jumpurl("http://vilshell.com")}>
                  <i className='iconfont'>&#xe7c4;</i>{i18n.t('Document')}</a>
                <a className='a2' onClick={()=>global.vildoc.jumpurl("http://vilshell.com")}>
                  <i className='iconfont'>&#xe637;</i>vilshell.com</a>
              </div>
            </div>
            <div className='area1_2'>
              {i18n.language=="zh_cn"&&<>
                <div className='area1_22'>有使用问题或功能建议，请微信(<a onClick={()=>Common.copyTextToClipboard('xianglou')}>xianglou</a>)或邮箱(<a onClick={()=>Common.copyTextToClipboard('kyle946@163.com')}>kyle946@163.com</a>)联系我们</div>
              </>}
              {i18n.language!="zh_cn"&&<>
                <div className='area1_22'>If you have any usage questions or feature suggestions, please contact us via WeChat (<a onClick={()=>Common.copyTextToClipboard('xianglou')}>xianglou</a>) or email (<a onClick={()=>Common.copyTextToClipboard('kyle946@163.com')}>kyle946@163.com</a>)</div>
              </>}
            </div>
          </div>
        </div>
      </>
    )
  }

}


export default LaunchHome;

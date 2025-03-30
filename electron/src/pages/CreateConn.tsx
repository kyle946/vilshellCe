
import * as React from 'react'
import { type PropsWithChildren, useState, useRef, useEffect } from 'react'
import * as ReactDOM from 'react-dom'
import i18n from 'i18next';
import fs from "fs";
import { createHashHistory, LocationDescriptorObject, History } from "history";
const history = createHashHistory();

import Joi from "joi";
import * as Common from "../components/Common";
import {
  Switch, RadioGroup, Radio, Form, Select, Tabs, TabPane, Button, Input, Spin, Toast
} from '@douyinfe/semi-ui';
import "./CreateConn.less"
import { yxulinux_connect_state, yxulinux_error_code as cc } from "../types/ErrorCode";
import * as dlg from "../components/Dialog";

import villib1 from "@/../dist/villib1.node";



type BtnSpinIProps = PropsWithChildren<{
  onClick: () => void,
  label: string,
  state: number,
  class: string
}>;
const BtnSpin: React.FC<BtnSpinIProps> = (props: BtnSpinIProps): React.ReactNode => {
  if (props.state == 0) {
    return <a className={props.class}><Spin size="middle" /></a>
  }
  else if (props.state == 1) {
    return <a className={props.class} onClick={props.onClick}>{props.label}</a>
  }
  else {
    return <a className={props.class}>{props.label}</a>
  }
}

/**************************************************
 * 
 * 
 * 
 * 
 * 
 * 
 **************************************************/

export default class CreateConn extends React.Component<any, any> {

  public hosts: any[];
  public refinput: React.RefObject<HTMLInputElement> = React.createRef<HTMLInputElement>();
  constructor(props: any) {
    super(props);
    this.hosts = [];
    this.startup = this.startup.bind(this);
    this.onopen = this.onopen.bind(this);
    this.onopen2 = this.onopen2.bind(this);
    this.verifyparam = this.verifyparam.bind(this);
    this.verifyhost = this.verifyhost.bind(this);
    this.onimportkey = this.onimportkey.bind(this);
    this.onclearkey = this.onclearkey.bind(this);
    this.onsuccess = this.onsuccess.bind(this);
    this.onKeyDown1 = this.onKeyDown1.bind(this);
    this.onClickNewConn = this.onClickNewConn.bind(this);
  }

  state = {
    id: 0,
    tips: "",
    conn_state: 1,
    host: "",
    port: 22,
    user: "",
    pwd: "",
    usepubkey: false,   //使用密钥登录
    privatekey: "",    //私钥文件
    remark: "",   //备注说明
    savepwd: false,       //保存密码(不推荐)
  }

  componentDidMount(): void {
    this.startup();
  }

  async startup() {
    let res = await global.vildoc.gethomepath("home");
    let id_rsa = res + global.DIRSEPA + ".ssh" + global.DIRSEPA + "id_rsa";
    if (fs.existsSync(id_rsa)) {
      this.setState({ privatekey: id_rsa });
    }
    setTimeout(() => {
      if(this.refinput.current){
        this.refinput.current.focus();
      }
    }, 100);
  }

  onopen() {
    if (!this.verifyparam()) {
      return;
    }
    if (this.state.usepubkey == false && this.state.pwd == "") {
      dlg.confirminput("Type password.", 3, "", (txt: string) => {
        this.setState({ pwd: txt }, () => {
          this.onopen2();
        });
        return true;
      })
    } else {
      this.onopen2();
    }
  }

  onopen2() {
    this.setState({ conn_state: 0 });
    global.vilssh.openconn(this.state).then((ret: number) => {
      if (ret == cc.YXU_SUCCESS) {
        // 连接成功
        this.setState({ conn_state: 1 });
        return true;
      }
      else if (ret == cc.YXU_KNOWN_HOSTS_UNKNOWN) {
        // 需要信任并更新主机密钥
        this.verifyhost();
        throw cc.YXU_SUCCESS;
      }
    }).then((res: any) => {
      return Common.afterSuccessfulConn();
    }).then((ret: number) => {
      this.onsuccess();
    }).catch((e: number) => {
      if (e == cc.YXU_SUCCESS) {
        //
      } else {
        // 连接失败后需要释放连接资源
        this.setState({ conn_state: 1, tips: i18n.t(`error_code_${e}`) });
        global.vilssh.releaseconn();
      }
    });
  }

  verifyhost() {
    let oncancel = () => { };
    let onconfirm = () => {
      this.setState({ conn_state: 0 });
      let ret = global.vilssh.hostupdate();
      if (ret != cc.YXU_SUCCESS) {
        this.setState({ conn_state: 1, tips: i18n.t(`error_code_${ret}`) });
        global.vilssh.releaseconn();
      }
      global.vilssh.openconn(this.state).then((ret: number) => {
        if (ret == cc.YXU_SUCCESS) {
          this.setState({ conn_state: 1 });
          return true;
        } else {
          throw ret;
        }
      }).then((res: any) => {
        return Common.afterSuccessfulConn();
      }).then((ret: number) => {
        this.onsuccess();
      }).catch((e: number) => {
        if (e == cc.YXU_SUCCESS) {

        } else {
          // 连接失败后需要释放连接资源
          this.setState({ conn_state: 1, tips: i18n.t(`error_code_${e}`) });
          global.vilssh.releaseconn();
        }
      });
    };
    dlg.confirm(i18n.t('tips'), "The server is unknown. Do you trust the host key?", oncancel, onconfirm, 6);
  }

  onimportkey() {
    global.vildoc.choosefile("Import Public key").then((res: any) => {
      if (!res) {
        throw "error";
      }
      if (res.length == 0) {
        throw "error"
      }
      let filepath = res[0];
      this.setState({ privatekey: filepath });
    }).catch((e: any) => {
      console.error(e);
    })
  }

  onclearkey() {
    this.setState({ privatekeyfile: "...", privatekey: "" });
  }


  // 验证参数
  verifyparam(): number {

    let indata = {
      id: this.state.id,
      host: this.state.host,
      port: this.state.port,
      user: this.state.user,
      pwd: this.state.savepwd ? this.state.pwd : "",
      usepubkey: +this.state.usepubkey,
      privatekey: this.state.privatekey,
      remark: this.state.remark,
    }

    const schema = Joi.object({
      id: Joi.number(),
      host: Joi.string().hostname().required().messages({
        'string.empty': '"Host address" is not allowed to be empty',
        'string.hostname': '"Host addres" must be a valid hostname'
      }),
      port: Joi.number().port().messages({
        'number.port': '"Port" must be a valid port',
      }),
      user: Joi.string().alphanum().min(3).max(30).required(),
      pwd: Joi.string().pattern(new RegExp(/^.{0,32}$/)).allow(''),
      usepubkey: Joi.number().allow(''),
      privatekey: Joi.string().allow(''),
      remark: Joi.string().pattern(new RegExp(/^.{0,64}$/)).allow(''),
    });

    const { error, value } = schema.validate(indata);
    if (error) {
      console.log(error.details);
      if ("details" in error) {
        this.setState({ tips: error.details[0].message });
      } else {
        this.setState({ tips: 'error' });
      }
      return 0;
    } else {
      return 1;
    }

  }

  onKeyDown1(e: React.KeyboardEvent) {
    if (e.key == 'Enter') {
      this.onopen();
      // e.preventDefault();
      // return;
    }
  }

  onsuccess() {
    this.props.onClose();
    let hoststring = this.state.user + '@' + this.state.host;
    global.ievt.emit("sethoststring", hoststring);
    history.replace({ pathname: "/main1" });
  }

  onClickNewConn() {
    this.setState({
      id: 0,
      host: '',
      port: 22,
      user: '',
      pwd: '',
      remark: '',
    });
  }

  renderCreateConn() {
    return (
      <div className='createconn'>
        {this.state.tips == "" ? <div className='line1 tips2'>&nbsp;</div> : <div className='line1 tips'>{this.state.tips}</div>}

        <div className='comset'>

          <div className='line1'>
            <div className='label1'>{i18n.t('Host addr')}</div>
            <div className='righttxt1'><Input defaultValue={this.state.host} ref={this.refinput} onChange={(v, e) => this.setState({ host: v })} /></div>
          </div>

          <div className='line1'>
            <div className='label1'>{i18n.t('Port')}</div>
            <div className='righttxt2'><Input defaultValue={this.state.port} onChange={(v, e) => this.setState({ port: v })} /></div>
          </div>

          <div className='line1'>
            <div className='label1'>{i18n.t('Use Private key')}</div>
            <div className='righttxt1'>
              <Switch defaultChecked={this.state.usepubkey} onChange={(c: boolean, e) => this.setState({ usepubkey: c })} />
              <div className='r11'>
                {this.state.privatekey == "" ? <>
                  <a className='selectfile' onClick={this.onimportkey}>Import Key</a>
                </> : <>
                  <a className='selectfile' onClick={this.onclearkey}>Clear Key</a>
                </>}
              </div>
              <div className='r12'>{this.state.privatekey}</div>
            </div>
          </div>

          <div className='line1'>
            <div className='label1'>{i18n.t('User')}</div>
            <div className='righttxt1'><Input defaultValue={this.state.user} onChange={(v, e) => this.setState({ user: v })} /></div>
          </div>

          <div className='line1'>
            <div className='label1'>{this.state.usepubkey ? i18n.t('Private Key password') : i18n.t('Password')}</div>
            <div className='righttxt1'>
              <Input
                mode='password'
                defaultValue={this.state.pwd}
                onKeyDown={this.onKeyDown1}
                onChange={(v, e) => this.setState({ pwd: v })} />
            </div>
          </div>

          <div className='line1'>
            <div className='label1'>{i18n.t('Notes')}</div>
            <div className='righttxt1'><Input defaultValue={this.state.remark} onChange={(v, e) => this.setState({ remark: v })} /></div>
          </div>

        </div>
        <div className='line1'>
          <div className='label1'></div>
          <div className='righttxt1'>
            <BtnSpin onClick={this.onopen} class='btn1 mr' label={i18n.t('Open')} state={this.state.conn_state} />
          </div>
        </div>
      </div>

    )
  }

  render(): React.ReactNode {
    return (
      <div className='CreateConn'>
        <div className='win'>
          <div className='closebtn'><a onClick={() => this.props.onClose()}><i className='iconfont'>&#xe8bb;</i></a></div>
          <div className='content'>
            <div className='titlearea'>
              <div className='title'>
                {i18n.t('New Connection')}
              </div>
              <div className='titlebtnarea'>
                <a className='aa2' onClick={this.onClickNewConn}>{i18n.t('New Conn')}</a>
              </div>
            </div>
            {this.renderCreateConn()}
          </div>
        </div>
      </div>
    )
  }
}



const closeconndlg = () => {
  try {
    ReactDOM.unmountComponentAtNode(document.getElementById('CreateConndlg'));
    document.body.removeChild(document.getElementById('CreateConndlg'));
  } catch (error) {
  }
}

const CreateConndlg = (label = 'modify_zindex') => {
  try {
    let _exists = document.getElementById('CreateConndlg');
    if (_exists) {
      Common.setdlgcss(_exists as HTMLDivElement, 'modify_zindex');  // 将窗口置顶
      return;
    }
    let ele = document.createElement("div");
    ele.id = "CreateConndlg";
    Common.setdlgcss(ele, label);  // 将窗口置顶
    document.body.appendChild(ele);
    ReactDOM.render(<CreateConn onClose={closeconndlg} />, ele);
  } catch (error) {
    console.error(error);
  }
}

export { closeconndlg, CreateConndlg }

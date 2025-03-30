
import * as React from 'react'
import * as ReactDOM from 'react-dom'
import i18n from 'i18next';
import {
  type PropsWithChildren,
  useState,
  useRef,
  useEffect
} from 'react'


import * as Common from "../components/Common";
import "./Dialog.less"

type AlertDlgIProps = PropsWithChildren<{
  title: string,
  txt: string,
  onclose?: () => void
}>
export function AlertDlg (props: AlertDlgIProps) {
  let _title = props.title ? props.title : "...";
  let _txt = props.txt ? props.txt : "...";
  function onclose() {
    if (props.onclose) {
      props.onclose();
    }
    try {
      ReactDOM.unmountComponentAtNode(document.getElementById('alertdlg'));
      document.body.removeChild(document.getElementById('alertdlg'));
    } catch (error) {
      console.error(error);
    }
  }
  return (
    <div className='AlertDlg'
      tabIndex={0} 
      onKeyDown={(e:React.KeyboardEvent)=>{
        console.log(e.key);
        if(e.key=="Enter"){
          onclose();
        }
        else if(e.key=="Escape"){
          onclose();
        }
      }}>
      <div className='bg'></div>
      <div className='content'>
        <div className='title'>{_title}</div>
        <div className='txt' dangerouslySetInnerHTML={{ __html: _txt }}></div>
        <div className='btn'>
          <a onClick={onclose}>{i18n.t('confirm')}</a>
        </div>
      </div>
    </div>
  )
}

type ConfirmDlgIProps = PropsWithChildren<{
  title: string,
  txt: string,
  timeout?: number,
  oncancel?: () => void,
  onconfirm?: () => void,
  cancel_text?: string,
  confirm_text?: string
}>
export function ConfirmDlg (props: ConfirmDlgIProps){

  let _title = props.title ? props.title : "...";
  let _txt = props.txt ? props.txt : "...";
  let _timeout = props.timeout ? props.timeout : 0;
  var [secshow, setsecshow] = useState(_timeout);
  let ref1 = useRef<HTMLDivElement>();

  var t2:NodeJS.Timeout;
  useEffect(()=>{
    ref1.current.focus();
    if(_timeout>0){
      t2=setInterval(()=>{
        if(secshow<=1){
          oncancel();
          clearInterval(t2);
          t2=null;
        }else{
          --secshow;
          setsecshow(secshow);
        }
      }, 1000);
    }
    // 卸载组件时停止计时事件
    return ()=>{
      clearInterval(t2);
      t2 = null;
    }
  });

  function oncancel() {
    if(t2){
      clearInterval(t2);
    }
    if (props.oncancel) {
      props.oncancel();
    }
    try {
      ReactDOM.unmountComponentAtNode(document.getElementById('confirmdlg'));
      document.body.removeChild(document.getElementById('confirmdlg'));
    } catch (error) {
      console.error(error);
    }
  }

  function onconfirm() {
    if (props.onconfirm) {
      props.onconfirm();
    }
    try {
      ReactDOM.unmountComponentAtNode(document.getElementById('confirmdlg'));
      document.body.removeChild(document.getElementById('confirmdlg'));
    } catch (error) {
      console.error(error);
    }
  }

  return (
    <div className='AlertDlg' 
    tabIndex={0} 
    onKeyDown={(e:React.KeyboardEvent)=>{
      if(e.key=="Enter"){
        onconfirm();
      }
      else if(e.key=="Escape"){
        oncancel();
      }
    }}
    ref={ref1}>
      <div className='bg'></div>
      <div className='content'>
        <div className='title'>{_title}</div>
        <div className='txt' dangerouslySetInnerHTML={{ __html: _txt }}></div>
        <div className='btn'>
          <a onClick={oncancel}>{props.cancel_text?props.cancel_text:i18n.t('cancel')} {secshow>0?"("+secshow+")":""}</a>
          <a onClick={onconfirm}>{props.confirm_text?props.confirm_text:i18n.t('confirm')}</a>
        </div>
      </div>
    </div>
  )
}



type ConfirmInputDlgIProps = PropsWithChildren<{
  title: string,
  type?: number,
  defaulttxt?: string,
  oncancel?: () => void,
  onconfirm?: (txt: string) => boolean
}>

/**
 * 确认弹窗，带输入框的
 * @param title 
 * @param type  输入模式： 1 正常，2 不允许输入回车，3 密码输入框， 4 单行输入框， 5 不允许输入回车和空格
 * @param onconfirm 
 * @returns 
 */
const ConfirmInputDlg: React.FC<ConfirmInputDlgIProps> = (props: ConfirmInputDlgIProps): React.ReactNode => {
  let _title = props.title ? props.title : "...";
  const [txt, settxt] = useState(props.defaulttxt || "");
  let inputref: React.RefObject<HTMLInputElement> = React.createRef<HTMLInputElement>();
  let textarearef: React.RefObject<HTMLTextAreaElement> = React.createRef<HTMLTextAreaElement>();
  function oncancel() {
    if (props.oncancel) {
      props.oncancel();
    }
    try {
      ReactDOM.unmountComponentAtNode(document.getElementById('confirminputdlg'));
      document.body.removeChild(document.getElementById('confirminputdlg'));
    } catch (error) {
      console.error(error);
    }
  }
  function onconfirm() {
    if (props.onconfirm) {
      // 如果返回true，则关闭窗口
      if (props.onconfirm(txt)) {
        try {
          ReactDOM.unmountComponentAtNode(document.getElementById('confirminputdlg'));
          document.body.removeChild(document.getElementById('confirminputdlg'));
        } catch (error) {
          console.error(error);
        }
      }
    }
  }
  function _onKeyDown1(e: React.KeyboardEvent) {
    if (e.key == 'Enter') {
      e.preventDefault();
      return;
    }
  }
  function _onKeyDown2(e: React.KeyboardEvent) {
    if (e.key == 'Enter' || e.key == ' ') {
      e.preventDefault();
      return;
    }
  }
  function _onKeyDown3(e: React.KeyboardEvent) {
    if (e.key == 'Enter') {
      onconfirm();
      // e.preventDefault();
      // return;
    }
  }
  useEffect(()=>{
    switch (props.type) {
      case 1:
      case 2:
      case 5:
        textarearef.current.focus();
        break;
      case 3:
      case 4:
        inputref.current.focus();
        break;
      default:
        break;
    }
  },[]);
  return (
    <div className='AlertDlg'>
      <div className='bg'></div>
      <div className='content'>
        <div className='title' dangerouslySetInnerHTML={{__html: _title}}></div>
        <div className='txt'>
          {(() => {
            let _node = <></>
            switch (props.type) {
              case 1:
                _node = <textarea ref={textarearef} onChange={e => settxt(e.target.value)}>{txt}</textarea>
                break;
              case 2:
                _node = <textarea ref={textarearef} onKeyDown={_onKeyDown1} onChange={e => settxt(e.target.value)}>{txt}</textarea>
                break;
              case 3:
                _node = <input ref={inputref} onKeyDown={_onKeyDown3} className='input1' type='password' onChange={e => settxt(e.target.value)} value={txt} />
                break;
              case 4:
                _node = <input ref={inputref} onKeyDown={_onKeyDown3} className='input1' type='text' onChange={e => settxt(e.target.value)} value={txt} />
                break;
              case 5:
                _node = <textarea ref={textarearef} onKeyDown={_onKeyDown2} onChange={e => settxt(e.target.value)}>{txt}</textarea>
                break;
              default:
                break;
            }
            return _node;
          })()}
        </div>
        <div className='btn2'>
          <a onClick={oncancel}>{i18n.t('cancel')}</a>
          <a onClick={onconfirm}>{i18n.t('confirm')}</a>
        </div>
      </div>
    </div>
  )
}


// component?: React.ComponentType<RouteComponentProps<any>> | React.ComponentType<any> | undefined;


type ConfirmCmpProps = PropsWithChildren<{
  title: string,
  componentfun: (refobj: React.RefObject<any>) => React.ReactNode,
  refobj: React.RefObject<any>,
  timeout?: number,
}>;
class ConfirmCmp extends React.Component<ConfirmCmpProps>{

  public timeout: number;
  public t2: NodeJS.Timeout;
  public secshow: number;

  constructor(props: ConfirmCmpProps){
    super(props);
    this.timeout = props.timeout ? props.timeout : 0;
    this.secshow = this.timeout;
    this.closeDlg = this.closeDlg.bind(this);
    this.onCancel = this.onCancel.bind(this);
    this.onConfirm = this.onConfirm.bind(this);
    this.state={
      rand: 0.1
    }
  }

  componentDidMount(): void {
    if(this.props.timeout>0){
      this.t2=setInterval(()=>{
        if(this.timeout<=1){
          this.closeDlg();
          clearInterval(this.t2);
          this.t2=null;
        }else{
          --this.timeout;
          this.setState({rand: Math.random()});
        }
      }, 1000);
    }
  }

  componentWillUnmount(): void {
    clearInterval(this.t2);
    this.t2 = null;
  }

  closeDlg(){
    if(this.t2){
      clearInterval(this.t2);
    }
    try {
      ReactDOM.unmountComponentAtNode(document.getElementById('confirmcmpdlg'));
      document.body.removeChild(document.getElementById('confirmcmpdlg'));
    } catch (error) {
      console.error(error);
    }
  }

  onCancel(e?: React.MouseEvent<HTMLAnchorElement>){
    this.closeDlg();
  }

  onConfirm(e?: React.MouseEvent<HTMLAnchorElement>){
    this.props.refobj.current.onConfirm().then(()=>this.closeDlg());
  }

  render(): React.ReactNode {
    return (
      <div className='ConfirmCmp1'>
        <div className='bg'></div>
        <div className='content'>
          <div className='closebtn'><a onClick={() => this.closeDlg()}><i className='iconfont'>&#xe8bb;</i></a></div>
          <div className='title' dangerouslySetInnerHTML={{__html: this.props.title}}></div>
          <div className='txt'>{this.props.componentfun(this.props.refobj)}</div>
          <div className='btn'>
            <a onClick={this.onCancel}>{i18n.t('cancel')}</a>
            <a onClick={this.onConfirm}>{i18n.t('confirm')}</a>
          </div>
        </div>
      </div>
    )
  }
}




type ConfirmCmp2Props = PropsWithChildren<{
  title: string,
  componentfun: (close:()=>void, setparam:(v:any)=>void, defaultParam:any) => React.ReactNode,
  setparam: (v:any)=>void,
  defaultParam: any,
  timeout?: number,
}>;
class ConfirmCmp2 extends React.Component<ConfirmCmp2Props>{

  public timeout: number;
  public t2: NodeJS.Timeout;

  constructor(props: ConfirmCmp2Props){
    super(props);
    //
    this.timeout = props.timeout ? props.timeout : 0;
    this.closeDlg = this.closeDlg.bind(this);
  }

  componentDidMount(): void {
    if(this.props.timeout>0){
      this.t2=setInterval(()=>{
        if(this.timeout<=1){
          this.closeDlg();
          clearInterval(this.t2);
          this.t2=null;
        }else{
          --this.timeout;
        }
      }, 1000);
    }
  }

  componentWillUnmount(): void {
    clearInterval(this.t2);
    this.t2 = null;
  }

  closeDlg(){
    if(this.t2){
      clearInterval(this.t2);
    }
    try {
      ReactDOM.unmountComponentAtNode(document.getElementById('confirmcmp2dlg'));
      document.body.removeChild(document.getElementById('confirmcmp2dlg'));
    } catch (error) {
      console.error(error);
    }
  }

  render(): React.ReactNode {
    return (
      <div className='ConfirmCmp1'>
        <div className='bg'></div>
        <div className='content'>
          <div className='closebtn'><a onClick={() => this.closeDlg()}><i className='iconfont'>&#xe8bb;</i></a></div>
          <div className='title' dangerouslySetInnerHTML={{__html: this.props.title}}></div>
          <div className='txt'>{this.props.componentfun(this.closeDlg, this.props.setparam, this.props.defaultParam)}</div>
        </div>
      </div>
    )
  }
}


/**
 * 提示弹窗
 * @param title 
 * @param body 
 * @param onclose 
 */
export function alert (title: string, body: string, onclose?: () => void) {
  let ele = document.createElement("div");
  ele.id = "alertdlg";
  Common.setdlgcss(ele, 'modify_zindex');  // 将窗口置顶
  ele.className = "dlg alert";
  document.body.appendChild(ele);
  ReactDOM.render(<AlertDlg title={title} txt={body} onclose={onclose} />, ele);
}

export function remove_alert () {
  // 移除其它窗口
  let dlgs: HTMLCollectionOf<Element> = document.getElementsByClassName("alert");
  if (dlgs.length > 0) {
    for (let i = 0; i < dlgs.length; i++) {
      let el = dlgs.item(i) as HTMLDivElement;
      document.body.removeChild(el);
    }
  }
}

export function remove_confirm () {
  // 移除其它窗口
  let dlgs: HTMLCollectionOf<Element> = document.getElementsByClassName("confirm");
  if (dlgs.length > 0) {
    for (let i = 0; i < dlgs.length; i++) {
      let el = dlgs.item(i) as HTMLDivElement;
      document.body.removeChild(el);
    }
  }
}

export function dlg_close_all () {
  // 移除其它窗口
  let dlgs: HTMLCollectionOf<Element> = document.getElementsByClassName("dlg");
  if (dlgs.length > 0) {
    for (let i = 0; i < dlgs.length; i++) {
      let el = dlgs.item(i) as HTMLDivElement;
      document.body.removeChild(el);
    }
  }
}

/**
 * 确认弹窗
 * @param title 
 * @param body 
 * @param oncancel 
 * @param onconfirm 
 */
export function confirm (title: string, body: string, oncancel?: () => void, onconfirm?: () => void, timeout?: number) {
  let ele = document.createElement("div");
  ele.id = "confirmdlg";
  Common.setdlgcss(ele, 'modify_zindex');  // 将窗口置顶
  ele.className = "dlg confirm";
  document.body.appendChild(ele);
  ReactDOM.render(<ConfirmDlg title={title} txt={body} oncancel={oncancel} onconfirm={onconfirm} timeout={timeout} />, ele);
}

/**
 * 确认弹窗
 * @param title 
 * @param body 
 * @param oncancel 
 * @param onconfirm 
 */
export function confirmbtntxt (title: string, body: string, cancel_text: string, confirm_text: string, oncancel?: () => void, onconfirm?: () => void, timeout?: number) {
  let ele = document.createElement("div");
  ele.id = "confirmdlg";
  Common.setdlgcss(ele, 'modify_zindex');  // 将窗口置顶
  ele.className = "dlg confirm";
  document.body.appendChild(ele);
  ReactDOM.render(<ConfirmDlg title={title} txt={body} oncancel={oncancel} cancel_text={cancel_text} confirm_text={confirm_text} onconfirm={onconfirm} timeout={timeout} />, ele);
}

/**
 * 确认弹窗，带输入框的
 * @param title 
 * @param type  输入模式： 1 正常，2 不允许输入回车，3 密码输入框， 4 单行输入框， 5 不允许输入回车和空格
 * @param onconfirm 
 */
export function confirminput (title: string, type: number, defaulttxt: string, onconfirm?: (txt: string) => boolean) {
  let _exists = document.getElementById('confirminputdlg');
  if (_exists) {
    Common.setdlgcss(_exists as HTMLDivElement, 'modify_zindex');  // 将窗口置顶
    return;
  }
  let ele = document.createElement("div");
  ele.id = "confirminputdlg";
  Common.setdlgcss(ele, 'modify_zindex');  // 将窗口置顶
  document.body.appendChild(ele);
  ReactDOM.render(<ConfirmInputDlg title={title} defaulttxt={defaulttxt} type={type} onconfirm={onconfirm} />, ele);
}


/**
 * 确认弹窗，带自定义组件的
 * @param title     标题
 * @param componentfun 返回自定义组件的函数 , 组件内必须有一个onConfirm方法，并且返回Promise
 * @param refobj 组件的ref引用对象
 * @param timeout 超时多少秒关闭窗口, 0为不超时
 * @example
    class CreateDatabase extends React.Component{
      constructor(props:any){
        super(props);
      }
      onConfirm(){
        return new Promise((r,j)=>{
          r(1);
        });
      }
      render(){
        return (
          <div>CreateDatabase</div>
        )
      }
    }
    let refobj = React.createRef<any>();
    dlg.confirmcmp(i18n.t("Modify Row"), (refobj:React.RefObject<any>)=><CreateDatabase ref={refobj} />, refobj, 0);
 * @returns 
 */
export function confirmcmp (title: string,  componentfun: (refobj: React.RefObject<any>)=>React.ReactNode, refobj: React.RefObject<any>, timeout: number ) {
  let _exists = document.getElementById('confirmcmpdlg');
  if (_exists) {
    Common.setdlgcss(_exists as HTMLDivElement, 'modify_zindex');  // 将窗口置顶
    return;
  }
  let ele = document.createElement("div");
  ele.id = "confirmcmpdlg";
  Common.setdlgcss(ele, 'modify_zindex');  // 将窗口置顶
  document.body.appendChild(ele);
  ReactDOM.render(<ConfirmCmp title={title} componentfun={componentfun} timeout={timeout} refobj={refobj} />, ele);
}



export function confirmcmp2 (
  title: string,  
  componentfun: (close:()=>void, setparam:(v:any)=>void, defaultParam:any)=>React.ReactNode, 
  setparam:(v:any)=>void, 
  defaultParam:any, timeout: number ) {
  let _exists = document.getElementById('confirmcmp2dlg');
  if (_exists) {
    Common.setdlgcss(_exists as HTMLDivElement, 'modify_zindex');  // 将窗口置顶
    return;
  }
  let ele = document.createElement("div");
  ele.id = "confirmcmp2dlg";
  Common.setdlgcss(ele, 'modify_zindex');  // 将窗口置顶
  document.body.appendChild(ele);
  ReactDOM.render(<ConfirmCmp2 title={title} componentfun={componentfun} setparam={setparam} defaultParam={defaultParam} timeout={timeout} />, ele);
}
import * as React from 'react'
import * as ReactDOM from 'react-dom'
import i18n from 'i18next';
import {
  type PropsWithChildren,
  useState,
  useRef,
  useEffect
} from 'react'
import * as Common from "./Common";
import "./ConfirmDialog.less"


const ConfirmDialogEleId = "ConfirmDialogEleId";

type ConfirmDialogIProps = PropsWithChildren<{
  title: string,
  txt: string,
  timeout?: number,
  btn_cancel_text: string,
  btn_yes_text: string,
  btn_confirm_text: string,
  oncancel?: () => void,
  onyes?: () => void
  onconfirm?: () => void
}>
const ConfirmDialog: React.FC<ConfirmDialogIProps> = (props: ConfirmDialogIProps): React.ReactNode => {

  let _title = props.title ? props.title : "...";
  let _txt = props.txt ? props.txt : "...";
  let _timeout = props.timeout ? props.timeout : 0;
  var [secshow, setsecshow] = useState(_timeout);

  let t2: NodeJS.Timeout;

  useEffect(() => {
    if (_timeout > 0) {
      t2 = setInterval(() => {
        if (secshow==1) {
          oncancel();
          clearInterval(t2);
          t2 = null;
        } else {
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

  const removedlg = ()=>{
    try {
      let _node = document.getElementById(ConfirmDialogEleId);
      if(_node){
        ReactDOM.unmountComponentAtNode(_node);
        document.body.removeChild(_node);
      }
    } catch (error) {
      console.error(error);
    }
  }

  const oncancel = ()=>{
    if (props.oncancel) {
      props.oncancel();
    }
    removedlg();
  }

  const onyes = ()=>{
    if (props.oncancel) {
      props.onyes();
    }
    removedlg();
  }

  const onconfirm = ()=>{
    if (props.onconfirm) {
      props.onconfirm();
    }
    removedlg();
  }

  return (
    <div className='ConfirmDialog'>
      <div className='bg'></div>
      <div className='content'>
        <div className='title'>{_title}</div>
        <div className='txt' dangerouslySetInnerHTML={{ __html: _txt }}></div>
        <div className='btn'>
          <a onClick={oncancel}>{props.btn_cancel_text} {secshow > 0 ? "(" + secshow + ")" : ""}</a>
          {props.btn_yes_text!=""&&<><a onClick={onyes}>{props.btn_yes_text}</a></>}
          <a onClick={onconfirm}>{props.btn_confirm_text}</a>
        </div>
      </div>
    </div>
  )
}


/**
 * 
 * @param title 标题
 * @param body 内容
 * @param btn_cancel_text 取消按钮文本
 * @param btn_yes_text 中是YES按钮文本，如果为空则不显示YES按钮
 * @param btn_confirm_text 确认按钮文本
 * @param oncancel 取消按钮事件
 * @param onyes YES按钮事件
 * @param onconfirm 确认按钮事件
 * @param timeout 超时关闭
 * @example confirmdlg( i18n.t('tips'), "你确定要清除记住的目录吗？", "Cancel", "", "Confirm", oncancel, onyes, onconfirm, 12);
 */
const confirmdlg = (
  title: string,
  body: string,
  btn_cancel_text: string,
  btn_yes_text: string,
  btn_confirm_text: string,
  oncancel?: () => void,
  onyes?: () => void,
  onconfirm?: () => void,
  timeout?: number
) => {
  let ele = document.createElement("div");
  ele.id = ConfirmDialogEleId;
  Common.setdlgcss(ele, 'modify_zindex');  // 将窗口置顶
  ele.className = "dlg ConfirmDialogEle";
  document.body.appendChild(ele);
  let _cmp = <ConfirmDialog
    title={title}
    txt={body}
    btn_cancel_text={btn_cancel_text}
    btn_yes_text={btn_yes_text}
    btn_confirm_text={btn_confirm_text}
    oncancel={oncancel}
    onyes={onyes}
    onconfirm={onconfirm}
    timeout={timeout} />;
  ReactDOM.render(_cmp, ele);
}



const rm_confirmdlg = () => {
  // 移除其它窗口
  let dlgs: HTMLCollectionOf<Element> = document.getElementsByClassName("ConfirmDialogEle");
  if (dlgs.length > 0) {
    for (let i = 0; i < dlgs.length; i++) {
      let el = dlgs.item(i) as HTMLDivElement;
      document.body.removeChild(el);
    }
  }
}

export { confirmdlg, rm_confirmdlg }

export default ConfirmDialog;
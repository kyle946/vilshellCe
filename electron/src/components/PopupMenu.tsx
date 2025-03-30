
import * as React from 'react'
import * as ReactDOM from 'react-dom'
import {
  type PropsWithChildren,
  useState,
  useRef,
  useEffect
} from 'react'

import * as Common from "../components/Common";
import "./PopupMenu.css"


type PopupMenuItem = {
  label: string,
  OnClick: ()=>void
}

type PopupMenuIProps = PropsWithChildren<{
  menudata: PopupMenuItem[],
}>

const PopupMenu: React.FC<PopupMenuIProps> = (props: PopupMenuIProps): React.ReactNode => {
  const onclick=(click:()=>void)=>{
    click();
    closePopupMenudlg();
  };
  return (
    <div className='PopupMenu'>
      <div className='submenu'>
        {props.menudata.map((menu,index)=>{
          return <a key={index} onClick={()=>onclick(menu.OnClick)}>{menu.label}</a>
        })}
      </div> 
    </div>
  )
}

const closePopupMenudlg = () => {
  try {
    let node=document.getElementById('PopupMenudlg');
    if(node){
      ReactDOM.unmountComponentAtNode(node);
      document.body.removeChild(document.getElementById('PopupMenudlg'));
    }
  } catch (error) {
    console.error(error);
  }
}

const PopupMenudlg = (ee: React.MouseEvent, menudata: PopupMenuItem[]) => {
  try {
    let _exists=document.getElementById('PopupMenudlg');
    if(_exists){
      return ;
    }
    let ele = document.createElement("div");
    ele.id = "PopupMenudlg";
    const menu_height = menudata.length*30;
    ele.className = "dlg";
    ele.style.zIndex = 10 + '';
    ele.style.position = "absolute";
    ele.style.top = (ee.clientY-10)+'px';
    ele.style.left = ee.clientX+'px';
    if((ee.clientY+menu_height)>document.body.offsetHeight){
      let _h = ee.clientY - menu_height;
      ele.style.top = (_h-40)+'px';
    }
    document.body.appendChild(ele);
    ReactDOM.render(<PopupMenu menudata={menudata} />, ele);
  } catch (error) {
    console.error(error);
  }
}

document.body.addEventListener("click",()=>closePopupMenudlg());

export { PopupMenudlg, closePopupMenudlg, type PopupMenuItem }


/*

弹出菜单、鼠标右键弹出菜单
example: 

import {PopupMenudlg, type PopupMenuItem} from "../components/PopupMenu"

const menudata: PopupMenuItem[] = [
  {label: "编辑文档", OnClick:()=>{}},
  {label: "添加子文档", OnClick:()=>{}},
  {label: "重命名", OnClick:()=>{}}
]
PopupMenudlg(e, menudata);

*/
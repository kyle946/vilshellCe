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
;
import "./Main1.less"




type Main1MenuProps = PropsWithChildren<{
  menu: object[],
  default: string,
  onclick: (label: string) => void,
  width?: number
}>;


const Main1Menu: React.FC<Main1MenuProps> = (props: Main1MenuProps) => {
  const [active, setactive] = useState(props.default);
  const onclick = (label: string) => {
    setactive(label);
    props.onclick(label);
  };
  const getclassname = (c: string) => {
    if (active == c) {
      return "active";
    }
  };
  useEffect(() => {
    global.ievt.addListener("onclickmenu", onclick);
  }, []);
  return (
    <div className='Main1Menu' style={props.width&&{width: props.width+"px"}}>
      {props.menu.map((v: any, index: number) => {
        return <a
          className={getclassname(v.label)}
          key={index}
          onClick={() => onclick(v.label)}>
          <i className="iconfont blue">{v.icon}</i>
          <span>{v.text}</span>
        </a>
      })}
    </div>
  )
};


export default Main1Menu;
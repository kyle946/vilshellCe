const path = require('path')
const fs = require('fs')
const webpack = require("webpack");
const HtmlWebpackPlugin = require('html-webpack-plugin')

// 获取项目根目录路径
const appDirectory = fs.realpathSync(path.resolve(__dirname, './'))
// 获取目标文件的函数
const resolveApp = relativePath => path.resolve(appDirectory, relativePath)


module.exports = [
  // 二、electron主进程
  {
    mode: process.env.NODE_ENV,
    entry: resolveApp("src/main.ts"),
    target: 'electron-main',
    output: {
      filename: 'main.js',
      path: resolveApp('dist'),
    },
    resolve: {
      extensions: ['.node', '.ts', '.tsx', '.js', '.jsx'],
      alias: {
        '@': resolveApp('src')
      }
    },
    module: {
      rules: [
        {
          test: /\.ts(x?)$/,
          include: /src/,
          use: [{ loader: 'ts-loader' }]
        },
        {
          test: /\.node$/,
          loader: "./loadnode.js",
        }
      ]
    }
  }
]

// //代码混淆
// if (process.env.NODE_ENV === 'production') {
//     module.exports.plugins = (module.exports.plugins || []).concat([new webpack.optimize.UglifyJsPlugin({
//         minimize: true,
//         compress: true
//     })]);
// }
const path = require('path')
const fs = require('fs')
const webpack = require("webpack");
const HtmlWebpackPlugin = require('html-webpack-plugin');

// 获取项目根目录路径
const appDirectory = fs.realpathSync(path.resolve(__dirname, './'))
// 获取目标文件的函数
const resolveApp = relativePath => path.resolve(appDirectory, relativePath)

module.exports = [
  // 渲染进程配置
  {
    mode: process.env.NODE_ENV,        //  development,  production
    target: 'electron-renderer',        //web, node, electron-renderer, electron-main
    node: {
      __dirname: false,
    },
    plugins: [
      new HtmlWebpackPlugin({
        template: resolveApp('src/index.html'), //指定模板路径
      })
    ],
    devServer: {
      host: '127.0.0.1',
      port: 8080,
      hot: true,
      headers: { 'Access-Control-Allow-Origin': '*' },
      static: [
        {
          directory: path.join(__dirname, 'dist'),
          publicPath: '/',
        }
      ]
    },
    // 入口文件
    entry: {
      app: resolveApp('src/index.tsx')
    },
    // 输出到dist文件夹, 文件名字为 bundle.js
    output: {
      filename: '[name].bundle.js',
      path: resolveApp('dist'),
    },
    resolve: {
      // 模块引入如果不加后缀名 根据以下后缀名顺序查找
      extensions: ['.node', '.ts', '.tsx', '.js', '.jsx'],
      alias: {
        '@': resolveApp('src')
      }
    },
    module: {
      rules: [
        {
          test: /\.(ts|tsx)$/,
          include: /src/,
          use: [
            {
              loader: 'ts-loader',
              options: { allowTsInNodeModules: true }
            }
          ]
        },
        {
          test: /\.css/,
          use: [
            {
              loader: 'style-loader'
            },
            'css-loader'
          ]
        },
        {
          test: /\.ttf$/,
          use: ['file-loader']
        },
        // 匹配后缀名为 .less 的模块
        {
          test: /\.less/,
          // loader 加载资源模块 类似工作管道 可以使用多个 loader 加载同一个资源模块 最终返回一段标准的 js 代码字符串（webpack 默认只支持 js 的语法）
          // 多个 loader 执行顺序是从后往前执行
          use: [
            // 把 css-loader 的处理结果 通过 style 标签 添加到 html 页面
            'style-loader',
            {
              loader: 'css-loader',
              options: {
                // 此处开启 在他前面的所有 loader 都必须开启
                sourceMap: true,
                // 在当前 loader 加载之前的 loader 数量
                importLoaders: 1
              }
            },
            {
              // 把 .less 模块的内容加载成 css
              loader: 'less-loader',
              options: {
                // 开启源码地图（可理解成源码中的错误定位）
                sourceMap: true
              }
            }
          ]
        },
        // 匹配图片资源模块
        {
          test: /\.(png|jpg|jpeg|gif)/,
          use: [
            {
              loader: 'file-loader',
              options: {
                name: '[name].[ext]',
                // 打包到 ouput.path + images 目录下 
                outputPath: 'images',
              }
            }
          ]
        },
        {
          test: /\.(ttf|eot|woff|woff2)$/,
          use: {
            loader: "file-loader",
            options: {
              name: "fonts/[name].[ext]",
            },
          },
        },
        {
          test: /\.node$/,
          loader: "./loadnode.js",
          options: {
            rewritePath: resolveApp('dist')
          }
        }
      ]
    }
  }
]
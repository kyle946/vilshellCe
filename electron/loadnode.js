var path = require("path");

module.exports = function(content) {
  const defaultConfig = {
    basePath: [],
    rewritePath: undefined,
    emit: true
  };

  const config = Object.assign(defaultConfig, this.query);
  const fileName = path.basename(this.resourcePath);

  if (config.emit) {
    if (this.emitFile) {
      this.emitFile(fileName, content, false);
    } else {
      throw new Error("emitFile function is not available");
    }
  }
  
  this.addDependency(this.resourcePath);

  if (config.rewritePath) {
    let filePath;
    if (config.rewritePath === "./" || config.rewritePath === ".\\") {
      filePath = JSON.stringify(config.rewritePath + fileName);
    } else {
      filePath = JSON.stringify(path.join(config.rewritePath, fileName));
    }
    return (
      "const path = require('path');" +
      "const filePath = path.resolve(path.dirname(global.process.execPath), " + filePath + ");" +
      "try { global.process.dlopen(module, filePath); } " +
      "catch(exception) { throw new Error('Cannot open ' + " +
      filePath +
      " + ': ' + exception); };"
    );
  } else {
    const filePathArray = config.basePath.concat(fileName);
    const filePath = JSON.stringify(filePathArray).slice(1, -1);
    return (
      "const path = require('path');" +
      "const filePath = path.resolve(__dirname, " +
      filePath +
      ");" +
      "try { global.process.dlopen(module, filePath); } " +
      "catch(exception) { throw new Error('Cannot open ' + filePath + ': ' + exception); };"
    );
  }
};

module.exports.raw = true;

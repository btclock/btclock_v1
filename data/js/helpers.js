Handlebars.registerHelper('splitText', function (aString) {
    if (aString.includes("/")) {
      var c = aString.split("/").map((el) => { return "<div class=\"flex-items\">" + el + "</div>"; }).join('');
      return "<div class=\"splitText\">" + c + "</div>";
    }
    if (aString.length == 0 || aString === " ") {
      aString = "&nbsp;&nbsp;";
    }
    //return aString;
    return "<div class=\"digit\">" + aString + "</div>";
  });

Handlebars.registerHelper('ifEquals', function(arg1, arg2, options) {
    return (arg1 == arg2) ? options.fn(this) : options.inverse(this);
});
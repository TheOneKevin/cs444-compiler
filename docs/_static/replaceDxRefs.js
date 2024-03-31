// On document load, replace all <em class="dxc"> with <a>
var dxc = document.querySelectorAll('em.dxc');
for (var i = 0; i < dxc.length; i++) {
    let className = dxc[i].textContent
    // Replace ':' by _1
    className = className.replace(/:/g, '_1');
    // Convert PascalCase to snake_case
    className = className.replace(/([A-Z])/g, "_$1").toLowerCase();
    // Make the link now
    const a = document.createElement('a');
    a.target = '_blank';
    a.href = `_static/doxygen-html/class${className}.html`;
    a.innerHTML = dxc[i].innerHTML;
    dxc[i].parentNode.replaceChild(a, dxc[i]);
}

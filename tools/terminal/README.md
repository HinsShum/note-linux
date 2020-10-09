# 解决terminal中使用git bash中文乱码的问题

​		使用terminal登录git bash时，登录命令如下：

```
path/bash.exe --login -i
```

​		使用`git status`乱码时，修改git配置如下：

```
git config --global core.quotepath false
```


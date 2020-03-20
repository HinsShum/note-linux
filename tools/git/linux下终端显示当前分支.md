```bash
# display branch name in git dir
function find_git_branch {
 local dir=. head
 until [ "$dir" -ef / ]; do
 if [ -f "$dir/.git/HEAD" ]; then
 head=$(< "$dir/.git/HEAD")
 if [[ $head == ref:\ refs/heads/* ]]; then
 git_branch="(${head#*/*/})"
 elif [[ $head != '' ]]; then
 git_branch='(detached)'
 else
 git_branch='(unknown)'
 fi
 return
 fi
 dir="../$dir"
 done
 git_branch=''
}
PROMPT_COMMAND="find_git_branch; $PROMPT_COMMAND"

PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\] $git_branch \$ '
```


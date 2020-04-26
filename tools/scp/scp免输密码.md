# SCP免输密码

```
#!/bin/sh

mkdir -p $HOME/.ssh
echo "ssh-rsa AAAAB3NzaC1yc2EAQABAAABAQDPJEc/+9dO/Wjklk/YK3Yd2S0X1UjY3kkkYYhbuqsO7k8i7mUoNPOYAcwZRyhr+X/7KPREFhyGsg8xMH0F/ZcKreLn3Owae6d+zcBoPE+2jwzZRBw51h8ZqgwllGudLnZPTs03TGCsGMrsHafT20R6De7QAtV7BHlRWx820chf6j3FKMv8VtPbhscwdRk0ug7UCWAefoMiLB7j/AIkesUgPn5TFgNLM4uuHmWYsKwlZenPcXUaGtppSdfnupiUXnXG8soo8ggRopceF9mkj+ba2qLvtkW/Bn4XAyVq2iGxxwpGU3yDnvNp9c6LOwkCdu6N7N0R8JVGnw0LrwtoVPx32gHuPl email@you.com" > $HOME/.ssh/authorized_keys
chown manage:root -R $HOME/.ssh
chmod 755 $HOME/.ssh
chmod 400 $HOME/.ssh/authorized_keys

```




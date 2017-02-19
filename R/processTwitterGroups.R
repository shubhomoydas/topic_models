

data <- read.csv("data/TwitterData/parsed/training-userGroups.csv",header=F)
allgroups <- unique(data[,c(1,3)])

minGroupSize <- 150
groups <- allgroups[allgroups[,2] > minGroupSize,]
sum(groups[,2])
nrow(groups)

hist(allgroups[,2],ylim=c(0,500))

filtered <- c()
for (i in 1:nrow(groups)) {
  filtered <- rbind(filtered, data[data[,1]==groups[i,1],])
}

#write.table(groups,file="data/TwitterData/parsed/filtered-userGroups.csv",sep=",",col.names=F,row.names=F,quote=F)

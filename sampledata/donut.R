
setwd("R/donut")

# Sample a donut

radius <- 90;
angles <- seq(from=-pi,to=pi,length.out=20)
x <- radius*cos(angles)
y <- radius*sin(angles)

#plot(x,y,typ="p")

px <- c()
py <- c()

for (i in 1:20) {
  sdx = 20
  sdy = 20
  if (i == 3) sdy = 80
  samplesx <- rnorm(20,mean=0,sd=sdx)
  samplesy <- rnorm(20,mean=0,sd=sdy)
  px <- c(px,x[i]+samplesx+250)
  py <- c(py,y[i]+samplesy+250)
}

png("Donut-shape.png")
plot(px,py,typ="p")
dev.off()

dataframe <- data.frame(x=px,y=py)
write.table(dataframe, file = "donut-shape.csv", append = FALSE, quote = FALSE, sep = ",",
            eol = "\n", na = "NA", dec = ".", row.names = FALSE,
            col.names = c("x","y"), qmethod = c("escape", "double"),
            fileEncoding = "")


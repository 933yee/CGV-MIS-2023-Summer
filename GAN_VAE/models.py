import torch
import torch.nn as nn
import torch.nn.functional as F
from padding_same_conv import Conv2d

# Decide which device we want to run on
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

class Encoder(nn.Module):
    def __init__(self, z_dim, ch=64):
        super(Encoder, self).__init__()
        self.ch = ch
        # (3, 64, 64)
        self.conv1 = Conv2d(3, ch, 5, stride=2)
        # (ch, 32, 32)
        self.conv2 = Conv2d(ch, ch*2, 5, stride=2)
        self.bn2 = nn.BatchNorm2d(ch*2)
        # TODO: Finish the model.
        self.conv3 = Conv2d(ch*2, ch*4, 3, stride=2)
        self.bn3 = nn.BatchNorm2d(ch*4)
        self.conv4 = Conv2d(ch*4, ch*8, 3, stride=2)

        self.fc1 = nn.Linear(ch*8*4*4, z_dim)
        self.fc2 = nn.Linear(ch*8*4*4, z_dim)

    def forward(self, x):
        # TODO: Finish the forward propagation.
        x = torch.relu(self.conv1(x))
        x = torch.relu(self.bn2(self.conv2(x)))
        x = torch.relu(self.bn3(self.conv3(x)))
        x = torch.relu(self.conv4(x))
        x = x.view(x.size(0), -1)
        # print(x.shape)
        z_mean = self.fc1(x)
        z_logvar = self.fc2(x)
        return z_mean, z_logvar

class Generator(nn.Module):
    def __init__(self, z_dim, ch=64):
        super(Generator, self).__init__()
        self.ch = ch
        self.fc1 = nn.Linear(z_dim, 8*8*ch*8)
        # (ch*8, 8, 8)
        self.conv1 = Conv2d(ch*8, ch*4, 3)
        # (ch*4, 8, 8)
        self.up2 = nn.Upsample(scale_factor=2, mode='nearest')
        self.conv2 = Conv2d(ch*4, ch*2, 3)
        self.bn2 = nn.BatchNorm2d(ch*2)
        # TODO: 
        self.up3 = nn.Upsample(scale_factor=2, mode='nearest')
        self.conv3 = Conv2d(ch*2, ch, 3)
        self.bn3 = nn.BatchNorm2d(ch)
        self.up4 = nn.Upsample(scale_factor=2, mode='nearest')
        self.conv4 = Conv2d(ch, 3, 3)

    def forward(self, z):
        # TODO
        x = self.fc1(z)
        x = x.view(-1, self.ch*8, 8, 8)
        x = F.relu(self.conv1(x))
        x = self.up2(x)
        x = F.relu(self.bn2(self.conv2(x)))
        x = self.up3(x)
        x = F.relu(self.bn3(self.conv3(x)))
        x = self.up4(x)
        x = self.conv4(x)
        return torch.sigmoid(x)
    
class Discriminator(nn.Module):
    def __init__(self, ch=64):
        super(Discriminator, self).__init__()
        self.ch = ch
        self.conv1 = Conv2d(3, ch, 4, stride=2)
        self.conv2 = Conv2d(ch, ch*2, 4, stride=2)
        self.bn2 = nn.BatchNorm2d(ch*2)
        # self.conv3 = Conv2d(ch*2, 1, 4)
        self.conv3 = Conv2d(ch*2, ch*4, 4, stride=2)
        self.conv4 = Conv2d(ch*4, ch*8, 4, stride=2)
        self.fc = nn.Linear(4*4*ch*8, 1)

    def forward(self, x):
        # TODO: Finish the forward propagation.
        x = F.relu(self.conv1(x))
        x = F.relu(self.conv2(x))
        x = F.relu(self.conv3(x))
        x = F.relu(self.conv4(x))
        # print(x.shape)
        x = x.view(-1, self.ch*8*4*4)
        x = self.fc(x)
        return torch.sigmoid(x)

# 64*64
# test = Discriminator()
# print(test(torch.randn(64, 3, 64, 64))[0].shape)

# test = Generator(128)
# print(test(torch.randn(64, 128)).shape)

# test = Encoder(128)
# print(test(torch.randn()))




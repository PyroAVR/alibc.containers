pipeline {
  agent {
    docker {
      image 'aperture_standard'
    }

  }
  stages {
    stage('configure') {
      steps {
        sh '''if [ -d build ];
then rm  -Rf build;
fi
mkdir build
meson build
'''
      }
    }
    stage('build') {
      steps {
        dir(path: 'build') {
          sh '''ninja
'''
        }

      }
    }
    stage('package') {
      parallel {
        stage('package') {
          steps {
            archiveArtifacts(artifacts: 'build/*', allowEmptyArchive: true, caseSensitive: true, onlyIfSuccessful: true)
          }
        }
        stage('test') {
          steps {
            dir(path: 'build') {
              sh '''ninja test
'''
            }

          }
        }
      }
    }
  }
}
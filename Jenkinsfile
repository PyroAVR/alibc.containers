pipeline {
  agent {
    docker {
      image 'aperture_standard'
    }

  }
  stages {
    stage('configure') {
      steps {
        sh '''mkdir -p build
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
      steps {
        archiveArtifacts(artifacts: '*', allowEmptyArchive: true, caseSensitive: true, onlyIfSuccessful: true)
      }
    }
  }
}
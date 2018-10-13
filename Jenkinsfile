pipeline {
  agent {
    docker {
      image 'aperture_standard'
    }

  }
  stages {
    stage('configure') {
      steps {
        sh '''mkdir build
meson build
'''
      }
    }
    stage('build') {
      steps {
        dir(path: 'build')
        sh '''ninja
'''
      }
    }
    stage('package') {
      steps {
        archiveArtifacts(artifacts: '*', allowEmptyArchive: true, caseSensitive: true, onlyIfSuccessful: true)
      }
    }
  }
}